//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STDINLINESTREAM_HPP
#define WEBSERV_FUTURES_STDINLINESTREAM_HPP

#include "../boxed/RcPtr.hpp"
#include "../futures/IStreamExt.hpp"
#include "../ioruntime/FileDescriptor.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include "../utils/utils.hpp"
#include "futures.hpp"
#include <fcntl.h>
#include <string>

using boxed::RcPtr;
using ioruntime::FileDescriptor;
using ioruntime::GlobalIoEventHandler;

namespace futures {

class FdLineStream : public IStreamExt<std::string> {

public:
    FdLineStream() = delete;
    explicit FdLineStream(int fd)
        : fd(fd)
    {
    }

    FdLineStream(FdLineStream&& other) noexcept
        : buffer(std::move(other.buffer))
        , rbuffer("")
        , head(other.head)
        , max(other.head)
        , fd(std::move(other.fd))
    {
        utils::mem_copy(rbuffer, other.rbuffer);
    }

    ~FdLineStream() override
    {
        if (fd.get_descriptor() >= 0)
            ;
    }

    StreamPollResult<std::string> poll_next(Waker&& waker) override
    {
        if (completed) {
            return StreamPollResult<std::string>::finished();
        }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        for (;;) {
            char c;
            GncResult res = get_next_character(c, Waker(waker));
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
                if (c == EOF) { // || (fd.get_descriptor() == STDIN_FILENO && std::cin.eof())) {
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

    GncResult get_next_character(char& c, Waker&& waker)
    {
        if (head == 0) {
            auto poll_result = fd.poll_read(rbuffer, sizeof(rbuffer), std::move(waker));

            if (poll_result.is_ready()) {
                auto result = poll_result.get();
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
    char rbuffer[64] {};
    int head = 0;
    int max = 0;
    FileDescriptor fd;
};
}

#endif //WEBSERV_FUTURES_STDINLINESTREAM_HPP
