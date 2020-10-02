//
// Created by boat on 9/30/20.
//

#include "SpanBody.hpp"

namespace ioruntime {

SpanBody::SpanBody(span<uint8_t> inner)
    : _inner(inner)
{
}

auto SpanBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto copy_me = _inner.first(buffer.size());
    _inner.remove_prefix_inplace(copy_me.size());

    if (copy_me.size() == 0) {
        return PollResult<IoResult>::ready(IoResult::eof());
    }
    utils::ft_memcpy(buffer.data(), copy_me.data(), copy_me.size());
    waker();
    return PollResult<IoResult>::ready(copy_me.size());
}

}