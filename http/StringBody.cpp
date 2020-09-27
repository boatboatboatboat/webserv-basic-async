//
// Created by boat on 8/22/20.
//

#include "StringBody.hpp"

#include <utility>

namespace http {

StringBody::StringBody(std::string body, bool stream_like)
    : stream_like(stream_like)
    , body(std::move(body))
{
}

PollResult<ssize_t> StringBody::poll_read(char* buffer, size_t size, Waker&& waker)
{
    auto left_to_write = std::min(size, body.length() - written);
    memcpy(buffer, body.c_str() + written, left_to_write);
    written += left_to_write;
    waker();
    if (stream_like) {
        if (left_to_write)
            return PollResult<ssize_t>::ready(left_to_write);
        else
            return PollResult<ssize_t>::pending();
    } else {
        return PollResult<ssize_t>::ready(left_to_write);
    }
}

}
