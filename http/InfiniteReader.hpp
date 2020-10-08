//
// Created by Djevayo Pattij on 8/23/20.
//

#ifndef WEBSERV_HTTP_INFINITEREADER_HPP
#define WEBSERV_HTTP_INFINITEREADER_HPP

#include "../ioruntime/IAsyncRead.hpp"

using ioruntime::IAsyncRead;
using ioruntime::IoResult;

class InfiniteReader : public IAsyncRead {
public:
    InfiniteReader() = default;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
};

#endif //WEBSERV_HTTP_INFINITEREADER_HPP
