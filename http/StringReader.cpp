//
// Created by boat on 8/22/20.
//

#include "StringReader.hpp"

#include <utility>

namespace http {

StringReader::StringReader(std::string body, bool stream_like)
    : stream_like(stream_like)
    , body(std::move(body))
{
}

auto StringReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto left_to_write = std::min(buffer.size(), body.length() - written);
    memcpy(buffer.data(), body.c_str() + written, left_to_write);
    written += left_to_write;
    waker();
    if (stream_like) {
        if (left_to_write)
            return PollResult<IoResult>::ready(left_to_write);
        else
            return PollResult<IoResult>::pending();
    } else {
        return PollResult<IoResult>::ready(left_to_write);
    }
}

}
