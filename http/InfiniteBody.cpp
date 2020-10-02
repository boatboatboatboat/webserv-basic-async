//
// Created by Djevayo Pattij on 8/23/20.
//

#include "InfiniteBody.hpp"

auto InfiniteBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    utils::ft_memset(buffer.data(), 'A', buffer.size());
    return PollResult<IoResult>::ready(buffer.size());
}
