//
// Created by boat on 8/23/20.
//

#ifndef WEBSERV_DEFAULTPAGEBODY_HPP
#define WEBSERV_DEFAULTPAGEBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "Status.hpp"

using ioruntime::IoResult;

namespace http {

[[maybe_unused]]
static const char* DEFAULT_PAGE_START = "<html><head><title>webserv</title></head><body><h1 style=\"text-align:center;\">";
[[maybe_unused]]
static const char* DEFAULT_PAGE_END = "</h1><hr><p style=\"text-align:center;\">webserv</p></body></html>";

class DefaultPageReader : public ioruntime::IAsyncRead {
public:
    DefaultPageReader() = delete;
    explicit DefaultPageReader(Status code);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    enum State {
        PageStart,
        Code,
        Space,
        Message,
        PageEnd
    };
    State state = PageStart;
    ssize_t written {};
    Status code;
};

}

#endif //WEBSERV_DEFAULTPAGEBODY_HPP
