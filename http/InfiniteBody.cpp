//
// Created by Djevayo Pattij on 8/23/20.
//

#include "InfiniteBody.hpp"
PollResult<ssize_t> InfiniteBody::poll_read(char* buffer, size_t size, Waker&& waker)
{
    utils::ft_memset(buffer, 'A', size);
    waker();
    return PollResult<ssize_t>::ready(size);
}
