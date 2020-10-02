//
// Created by Djevayo Pattij on 8/23/20.
//

#ifndef WEBSERV_HTTP_INFINITEBODY_HPP
#define WEBSERV_HTTP_INFINITEBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"

using ioruntime::IAsyncRead;
using ioruntime::IoResult;

class InfiniteBody : public IAsyncRead {
public:
    InfiniteBody() = default;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
};

#endif //WEBSERV_HTTP_INFINITEBODY_HPP
