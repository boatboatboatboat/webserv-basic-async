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
class FdLineStream : public IStream<std::string> {
    class SetReadyFunctor : public Functor {
    public:
        SetReadyFunctor(RcPtr<Mutex<bool>> cr_source)
            : cread(cr_source)
        {
        }
        ~SetReadyFunctor() { }
        void operator()() override
        {
            auto cread_guard = cread->lock();
            *cread_guard = true;
            //  DBGPRINT("SetReady called");
        }

    private:
        // fixme: there might be a datarace during dtor of the real mutex
        RcPtr<Mutex<bool>> cread;
    };

public:
    FdLineStream() = delete;
    explicit FdLineStream(int fd)
        : can_read_mutex(Mutex(false))
        , buffer("")
        , fd(fd)
    {
        BoxFunctor ptr = BoxFunctor(new SetReadyFunctor(RcPtr(can_read_mutex)));
        GlobalIoEventHandler::register_reader_callback(fd, std::move(ptr));
        // TODO: err handling for fcntl
        fcntl(fd, F_SETFL, O_NONBLOCK);
    }
    FdLineStream(FdLineStream&& other)
        : can_read_mutex(std::move(other.can_read_mutex))
        , buffer(std::move(other.buffer))
        , rbuffer("")
        , head(other.head)
        , max(other.head)
        , fd(other.fd)
    {
        util::mem_copy(rbuffer, other.rbuffer);
        other.fd = -1;
        GlobalIoEventHandler::unregister_reader_callbacks(fd);
        BoxFunctor ptr = BoxFunctor(new SetReadyFunctor(RcPtr(can_read_mutex)));
        GlobalIoEventHandler::register_reader_callback(fd, std::move(ptr));
    }
    ~FdLineStream() override
    {
        if (fd >= 0)
            GlobalIoEventHandler::unregister_reader_callbacks(fd);
        DBGPRINT("FLS dtor");
    }
    StreamPollResult<std::string> poll_next(Waker&& waker)
    {
        if (completed) {
            DBGPRINT("FLS completed");
            return StreamPollResult<std::string>::finished();
        }
        if (!waker_set) {
            GlobalIoEventHandler::register_reader_callback(fd, waker.boxed());
        }
        bool can_read;
        {
            auto guard = can_read_mutex->lock();
            can_read = *guard;
        }
        if (can_read) {
            for (;;) {
                char c;
                int res = get_next_character(c);
                switch (res) {
                case 0: { // An error has occurred
                    // TODO: error handling
                    throw "FdLineStream poll_next Res error";
                } break;
                case 1: { // A character was returned
                    // EOF can't be reached on non-blocking files
                    // but according to the POSIX standard
                    // all regular files ignore the non-blocking flag
                    // and will behave as if they were blocking
                    // However they will always return "ready" in select
                    // which means that they can return EOF even when in select.
                    //
                    // We also check for stdin having reached EOF.
                    if (c == EOF || (fd == STDIN_FILENO && std::cin.eof())) {
                        // If there's nothing in the buffer, stop the stream immediately.
                        // Otherwise, set an internal completion flag and re-run,
                        // so we can return the contents of the buffer.
                        if (head == 0) {
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
                case 2: { // Buffer read depleted
                    return StreamPollResult<std::string>::pending();
                } break;
                }
            }
        } else {
            return StreamPollResult<std::string>::pending();
        }
    }

private:
    // fixme: the result of this method should probably be an enum
    int get_next_character(char& c)
    {
        if (head == 0) {
            bool can_read;
            {
                auto can_read_guard = can_read_mutex->lock();
                can_read = *can_read_guard;
                if (*can_read_guard) {
                    *can_read_guard = false;
                }
            }

            if (can_read) {
                if ((max = read(fd, rbuffer, sizeof(rbuffer))) < 0) {
                    return (0);
                }
            } else {
                return (2);
            }
        }
        head += 1;
        c = max == 0 ? (char)EOF : rbuffer[head - 1];
        if (head == max)
            head = 0;
        return (1);
    }
    bool completed = false;
    bool waker_set = false;
    RcPtr<Mutex<bool>> can_read_mutex;
    std::string buffer;
    char rbuffer[64];
    int head = 0;
    int max = 0;
    int fd;
};
}

#endif //WEBSERV_FUTURES_STDINLINESTREAM_HPP
