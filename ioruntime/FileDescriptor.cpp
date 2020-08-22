//
// Created by Djevayo Pattij on 8/21/20.
//

#include "FileDescriptor.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include <fcntl.h>

namespace ioruntime {

FileDescriptor::SetReadyFunctor::SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source)
    : ready_mutex(std::move(cr_source))
{
}

void FileDescriptor::SetReadyFunctor::operator()()
{
    auto cread_guard = ready_mutex->lock();
    *cread_guard = true;
}

FileDescriptor::FileDescriptor(int fd)
{
    if (fd < 0)
        throw std::runtime_error("FileDescriptor: ctor: bad file descriptor passed");
    int nonblocking_error = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (nonblocking_error)
        throw std::runtime_error("FileDescriptor: ctor: failed to set fd as nonblocking");
    descriptor = fd;
    BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
    BoxFunctor write_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(read_cb), true);
    GlobalIoEventHandler::register_writer_callback(descriptor, std::move(write_cb), true);
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

PollResult<ssize_t> FileDescriptor::poll_read(char* buffer, size_t size, Waker&& waker)
{
    auto ready_guard = ready_to_read->lock();
    if (!*ready_guard) {
        // Register the passed waker so the parent future can be woken up
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), false, 10);
        // File descriptor is not ready to read,
        // return that it's pending
        return PollResult<ssize_t>::pending();
    }
    ssize_t result = read(buffer, size);

    // Re-register the "ready" callback
    BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);

    // Register the passed waker so the parent future can be woken up
    GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), false, 10);

    // Set the file descriptor as not ready
    *ready_guard = false;

    // Return the result of read
    // the + 0 fixes an error related to result being an lvalue
    return PollResult<ssize_t>::ready(result + 0);
}

PollResult<ssize_t> FileDescriptor::poll_write(const char* buffer, size_t size, Waker&& waker)
{
    auto ready_guard = ready_to_write->lock();
    if (!*ready_guard) {
        // Register the passed waker so the parent future can be woken up
        GlobalIoEventHandler::register_writer_callback(descriptor, waker.boxed(), false, 10);

        // File descriptor is not ready to write,
        // return that it's pending
        return PollResult<ssize_t>::pending();
    }
    ssize_t result = write(buffer, size);

    // Re-register the "ready" callback
    BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
    GlobalIoEventHandler::register_writer_callback(descriptor, std::move(read_cb), true);

    // Register the passed waker so the parent future can be woken up
    GlobalIoEventHandler::register_writer_callback(descriptor, waker.boxed(), false, 10);

    // Set the file descriptor as not ready
    *ready_guard = false;

    // Return the result of write
    // the + 0 fixes an error related to result being an lvalue
    return PollResult<ssize_t>::ready(result + 0);
}

int FileDescriptor::get_descriptor() const
{
    return descriptor;
}
ssize_t FileDescriptor::read(char* buffer, size_t size)
{
    return ::read(descriptor, buffer, size);
}
ssize_t FileDescriptor::write(const char* buffer, size_t size)
{
    return ::write(descriptor, buffer, size);
}

FileDescriptor::FileDescriptor():
    descriptor(-1),
    ready_to_read(RcPtr<Mutex<bool>>::uninitialized()),
    ready_to_write(RcPtr<Mutex<bool>>::uninitialized())
{
}

FileDescriptor FileDescriptor::uninitialized()
{
    return FileDescriptor();
}

int FileDescriptor::close() &&
{
    auto fd = std::move(*this);
    return (0);
}

}
