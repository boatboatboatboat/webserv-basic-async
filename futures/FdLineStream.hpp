//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STDINLINESTREAM_HPP
#define WEBSERV_FUTURES_STDINLINESTREAM_HPP

#include "../boxed/RcPtr.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include "../util/mem_copy.hpp"
#include "../util/util.hpp"
#include "futures.hpp"
#include <fcntl.h>
#include <string>

using boxed::RcPtr;
using ioruntime::GlobalIoEventHandler;

namespace futures {

class FileDescriptor {
    class SetReadyFunctor : public Functor {
    public:
        SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source)
            : ready_mutex(std::move(cr_source))
        {
        }
        ~SetReadyFunctor() {}
        void operator()() override
        {
            auto cread_guard = ready_mutex->lock();
            *cread_guard = true;
        }

    private:
        RcPtr<Mutex<bool>> ready_mutex;
    };

public:
    FileDescriptor() = delete;
    explicit FileDescriptor(int fd)
    {
        if (fd < 0)
            throw std::runtime_error("FileDescriptor: ctor: bad file descriptor passed");
        int nonblocking_error = fcntl(fd, F_SETFL, O_NONBLOCK);
        if (nonblocking_error)
            throw std::runtime_error("FileDescriptor: ctor: failed to set fd as nonblocking");
        descriptor = fd;
        BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
        BoxFunctor write_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(read_cb), true, 0);
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(write_cb), true, 0);
    }
    FileDescriptor(FileDescriptor&& other)
        : ready_to_read(std::move(other.ready_to_read))
        , ready_to_write(std::move(other.ready_to_write))
        , descriptor(other.descriptor)
    {
        other.descriptor = -1;
    }
    ~FileDescriptor()
    {
        if (descriptor >= 0) {
            GlobalIoEventHandler::unregister_reader_callbacks(descriptor);
            GlobalIoEventHandler::unregister_writer_callbacks(descriptor);
        }
    }
    bool read_from(char* buffer, size_t size, size_t& read_result)
    {
        auto ready_guard = ready_to_read->lock();
        if (!*ready_guard) {
            return false;
        }
        read_result = read(descriptor, buffer, size);
        BoxFunctor read_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_read)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(read_cb), true, 0);
        *ready_guard = false;
        return true;
    }

    bool write_to(char* buffer, size_t size, size_t& write_result)
    {
        auto ready_guard = ready_to_write->lock();
        if (!*ready_guard) {
            return false;
        }
        write_result = read(descriptor, buffer, size);
        BoxFunctor write_cb = BoxFunctor(new SetReadyFunctor(RcPtr(ready_to_write)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(write_cb), true, 0);
        *ready_guard = false;
        return true;
    }

    int get_descriptor() const
    {
        return descriptor;
    }

private:
    int descriptor;
    RcPtr<Mutex<bool>> ready_to_read = RcPtr(Mutex(false));
    RcPtr<Mutex<bool>> ready_to_write = RcPtr(Mutex(false));
};

class FdLineStream : public IStream<std::string> {

public:
    FdLineStream() = delete;
    explicit FdLineStream(int fd)
        : buffer("")
        , fd(fd)
    {
    }

    FdLineStream(FdLineStream&& other)
        : buffer(std::move(other.buffer))
        , rbuffer("")
        , head(other.head)
        , max(other.head)
        , fd(std::move(other.fd))
    {
        DBGPRINT("FLS Moved");
        util::mem_copy(rbuffer, other.rbuffer);
    }

    ~FdLineStream() override
    {
        DBGPRINT("FLS Dtor");
    }

    StreamPollResult<std::string> poll_next(Waker&& waker) override
    {
        if (completed) {
            DBGPRINT("FLS completed");
            return StreamPollResult<std::string>::finished();
        }
        GlobalIoEventHandler::register_reader_callback(fd.get_descriptor(), waker.boxed(), true, 1);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        for (;;) {
            char c;
            GncResult res = get_next_character(c);
            switch (res) {
            case Error: { // An error has occurred
                // TODO: error handling
                throw std::runtime_error("FdLineStream poll_next res error");
            } break;
            case CharacterReturned: { // A character was returned
                // EOF can't be reached on non-blocking files
                // but according to the POSIX standard
                // all regular files ignore the non-blocking flag
                // and will behave as if they were blocking
                // However they will always return "ready" in select
                // which means that they can return EOF even when in select.
                //
                // We also check for stdin having reached EOF.
                if (c == EOF || (fd.get_descriptor() == STDIN_FILENO && std::cin.eof())) {
                    DBGPRINT("eof reached1");
                    // If there's nothing in the buffer, stop the stream immediately.
                    // Otherwise, set an internal completion flag and re-run,
                    // so we can return the contents of the buffer.
                    if (head == 0) {
                        DBGPRINT("eof reached");
                        return StreamPollResult<std::string>::finished();
                    } else {
                        completed = true;
                        waker();
                        return StreamPollResult<std::string>::ready(std::move(buffer));
                    }
                }
                if (c == '\n') {
                    std::string out = std::move(buffer);
                    buffer = "";
                    return StreamPollResult<std::string>::ready(std::move(out));
                }
                buffer += c;
            } break;
            case NotReady: { // Buffer read depleted
                return StreamPollResult<std::string>::pending();
            } break;
            default: {
                throw std::runtime_error("FdLineStream: poll_next: unreachable");
            } break;
            }
        }
#pragma clang diagnostic pop
    }

private:
    enum GncResult {
        Error,
        CharacterReturned,
        NotReady
    };

    // fixme: the result of this method should probably be an enum
    GncResult get_next_character(char& c)
    {
        if (head == 0) {
            size_t result;

            if (fd.read_from(rbuffer, sizeof(rbuffer), result)) {
                if (result < 0) {
                    // fixme: Errno is checked after a read, non-compliant to subject
                    std::stringstream x;
                    x << "Read error: " << strerror(errno);
                    DBGPRINT(x.str());
                    return (Error);
                }
                max = result;
            } else {
                return (NotReady);
            }
        }
        head += 1;
        c = max == 0 ? (char)EOF : rbuffer[head - 1];
        if (head == max)
            head = 0;
        return (CharacterReturned);
    }

    bool completed = false;
    std::string buffer;
    char rbuffer[64];
    int head = 0;
    int max = 0;
    FileDescriptor fd;
};
}

#endif //WEBSERV_FUTURES_STDINLINESTREAM_HPP
