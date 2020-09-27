//
// Created by Djevayo Pattij on 8/23/20.
//

#ifndef WEBSERV_HTTP_INFINITEBODY_HPP
#define WEBSERV_HTTP_INFINITEBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"

using ioruntime::IAsyncRead;

class InfiniteBody : public IAsyncRead {
public:
    InfiniteBody() = default;
    PollResult<ssize_t> poll_read(char* buffer, size_t size, Waker&& waker) override;
};

#endif //WEBSERV_HTTP_INFINITEBODY_HPP
