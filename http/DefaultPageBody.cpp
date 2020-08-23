//
// Created by boat on 8/23/20.
//

#include "DefaultPageBody.hpp"

namespace http {

DefaultPageBody::DefaultPageBody(HttpStatus code)
    : code(code)
{
}

PollResult<ssize_t> DefaultPageBody::poll_read(char* buffer, size_t size, Waker&& waker)
{
    //FIXME: use libft strlen
    char buf[16];
    char const* str;
    size_t len;
    switch (state) {
    case PageStart: {
        str = DEFAULT_PAGE_START;
        len = std::strlen(DEFAULT_PAGE_START);
    } break;
    case Code: {
        std::sprintf(buf, "%u", code.code);
        str = buf;
        len = std::strlen(buf);
    } break;
    case Space: {
        str = " ";
        len = 1;
    } break;
    case Message: {
        str = code.message;
        len = std::strlen(code.message);
    } break;
    case PageEnd: {
        str = DEFAULT_PAGE_END;
        len = std::strlen(DEFAULT_PAGE_END);
    } break;
    }
    auto left_to_write = std::min(size, len - written);
    memcpy(buffer, str + written, left_to_write);
    written += left_to_write;
    if (left_to_write == 0) {
        if (state != PageEnd) {
            written = 0;
            // the enums are consecutive, we can just increase by one
            state = static_cast<State>(static_cast<int>(state) + 1);
            return poll_read(buffer + written, size - written, std::move(waker));
        }
    }
    waker();
    return PollResult<ssize_t>::ready(left_to_write);
}

}