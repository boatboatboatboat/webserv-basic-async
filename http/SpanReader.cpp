//
// Created by boat on 9/30/20.
//

#include "SpanReader.hpp"

namespace ioruntime {

SpanReader::SpanReader(const span<uint8_t> inner)
    : _inner(inner)
{
}

auto SpanReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto copy_me = _inner.first(buffer.size());
    _inner.remove_prefix_inplace(copy_me.size());

    if (copy_me.size() == 0) {
        return PollResult<IoResult>::ready(IoResult::eof());
    }
    _bytes_read += copy_me.size();
    utils::ft_memcpy(buffer.data(), copy_me.data(), copy_me.size());
    waker();
    return PollResult<IoResult>::ready(copy_me.size());
}

void SpanReader::discard_first(size_t discardable)
{
    _inner.remove_prefix_inplace(discardable);
}

auto SpanReader::get_bytes_read() const -> size_t
{
    return _bytes_read;
}

}