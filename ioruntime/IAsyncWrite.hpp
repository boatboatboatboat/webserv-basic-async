//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_IASYNCWRITE_HPP
#define WEBSERV_IORUNTIME_IASYNCWRITE_HPP

#include "../futures/PollResult.hpp"
#include <algorithm>

using futures::PollResult;

namespace ioruntime {

class IAsyncWrite {
public:
    virtual auto poll_write(const char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCWRITE_HPP
