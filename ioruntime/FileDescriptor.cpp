//
// Created by Djevayo Pattij on 8/21/20.
//

#include "FileDescriptor.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include <fcntl.h>

namespace ioruntime {

FileDescriptor::FileDescriptor(int fd)
{
    try {
        if (fd < 0)
            throw std::runtime_error("FileDescriptor: ctor: bad file descriptor passed");
        int nonblocking_error = fcntl(fd, F_SETFL, O_NONBLOCK);
        if (nonblocking_error) {
            throw std::runtime_error("FileDescriptor: ctor: failed to set fd as nonblocking");
        }
        descriptor = fd;
        BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
        BoxFunctor write_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(read_cb), true);
        GlobalIoEventHandler::register_writer_callback(descriptor, std::move(write_cb), true);
    } catch (std::exception& e) {
        // Destructors are only called for the completely constructed objects.
        // When constructor of an object throws an exception, destructor for that object is not called.

        // Manually close the file descriptor - otherwise it will leak.
        ::close(fd);

        // Rethrow the exception.
        throw;
    }
}

FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : descriptor(other.descriptor)
    , ready_to_read(std::move(other.ready_to_read))
    , ready_to_write(std::move(other.ready_to_write))
{
    other.descriptor = -1;
}

FileDescriptor::~FileDescriptor()
{
    if (descriptor >= 0) {
        GlobalIoEventHandler::unregister_reader_callbacks(descriptor);
        GlobalIoEventHandler::unregister_writer_callbacks(descriptor);
        ::close(descriptor);
    }
}

auto FileDescriptor::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto ready_guard = ready_to_read->lock();
    if (!*ready_guard) {
        // Register the passed waker so the parent future can be woken up
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 10);
        // File descriptor is not ready to read,
        // return that it's pending
        return PollResult<IoResult>::pending();
    }
    ssize_t result = read(buffer.data(), buffer.size());

    // Re-register the "ready" callback
    BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);

    // Register the passed waker so the parent future can be woken up
    GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 10);

    // Set the file descriptor as not ready
    *ready_guard = false;

    // Return the result of read
    return PollResult<IoResult>::ready(IoResult(result));
}

auto FileDescriptor::poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto ready_guard = ready_to_write->lock();
    if (!*ready_guard) {
        // Register the passed waker so the parent future can be woken up
        GlobalIoEventHandler::register_writer_callback(descriptor, waker.boxed(), true, 10);

        // File descriptor is not ready to write,
        // return that it's pending
        return PollResult<IoResult>::pending();
    }
    ssize_t result = write(buffer.data(), buffer.size());

    if (result < 0) {
        TRACEPRINT("write error: " << strerror(errno));
    }

    // Re-register the "ready" callback
    BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
    GlobalIoEventHandler::register_writer_callback(descriptor, std::move(read_cb), true);

    // Register the passed waker so the parent future can be woken up
    GlobalIoEventHandler::register_writer_callback(descriptor, waker.boxed(), true, 10);

    // Set the file descriptor as not ready
    *ready_guard = false;

    // Return the result of write
    return PollResult<IoResult>::ready(IoResult(result));
}

auto FileDescriptor::get_descriptor() const -> int
{
    return descriptor;
}

auto FileDescriptor::read(void* buffer, size_t size) -> ssize_t
{
    return ::read(descriptor, buffer, size);
}

auto FileDescriptor::write(void const* buffer, size_t size) -> ssize_t
{
    return ::write(descriptor, buffer, size);
}

FileDescriptor::FileDescriptor()
    : descriptor(-1)
    , ready_to_read(RcPtr<Mutex<bool>>::uninitialized())
    , ready_to_write(RcPtr<Mutex<bool>>::uninitialized())
{
}

auto FileDescriptor::close() && -> int
{
    auto fd = std::move(*this);
    return (0);
}

auto FileDescriptor::can_read() -> bool
{
    auto read_ready = ready_to_read->lock();
    return *read_ready;
}

auto FileDescriptor::can_write() -> bool
{
    auto write_ready = ready_to_write->lock();
    return *write_ready;
}

}
