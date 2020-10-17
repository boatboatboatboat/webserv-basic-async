//
// Created by boat on 8/22/20.
//

#include "StringReader.hpp"

#include <utility>

namespace http {

StringReader::StringReader(std::string body, bool stream_like, size_t fake_cap)
    : stream_like(stream_like)
    , fake_cap(fake_cap)
    , body(std::move(body))
{
}

auto StringReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto max_size = buffer.size();
    if (fake_cap != 0) {
        max_size = std::min(fake_cap, buffer.size());
    }
    auto left_to_write = std::min(max_size, body.length() - written);
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
