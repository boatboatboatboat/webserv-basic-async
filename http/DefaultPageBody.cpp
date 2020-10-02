//
// Created by boat on 8/23/20.
//

#include "DefaultPageBody.hpp"

namespace http {

DefaultPageBody::DefaultPageBody(Status code)
    : code(code)
{
}

auto DefaultPageBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    char buf[8];
    std::string_view str;
    switch (state) {
    case PageStart: {
        str = DEFAULT_PAGE_START;
    } break;
    case Code: {
        std::sprintf(buf, "%u", code.code);
        str = buf;
    } break;
    case Space: {
        str = " ";
    } break;
    case Message: {
        str = code.message;
    } break;
    case PageEnd: {
        str = DEFAULT_PAGE_END;
    } break;
    }
    auto len = str.length();
    auto left_to_write = std::min(buffer.size(), len - written);
    utils::ft_memcpy(buffer.data(), str.data() + written, left_to_write);
    written += left_to_write;
    if (left_to_write == 0) {
        if (state != PageEnd) {
            auto oldw = written;
            written = 0;
            // the enums are consecutive, we can just increase by one
            state = static_cast<State>(static_cast<int>(state) + 1);
            return poll_read(buffer.remove_prefix(oldw), std::move(waker));
        }
    }
    waker();
    return PollResult<IoResult>::ready(left_to_write);
}

}