//
// Created by boat on 8/23/20.
//

#ifndef WEBSERV_DEFAULTPAGEBODY_HPP
#define WEBSERV_DEFAULTPAGEBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "HttpStatus.hpp"

using ioruntime::IoResult;

namespace http {

static const char* DEFAULT_PAGE_START = "<html><head><title>webserv</title></head><body><h1 style=\"text-align:center;\">";
static const char* DEFAULT_PAGE_END = "</h1><hr><p style=\"text-align:center;\">webserv</p></body></html>";

class DefaultPageBody : public ioruntime::IAsyncRead {
public:
    DefaultPageBody() = delete;
    explicit DefaultPageBody(HttpStatus code);
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
    HttpStatus code;
};

}

#endif //WEBSERV_DEFAULTPAGEBODY_HPP
