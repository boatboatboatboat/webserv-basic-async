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
    virtual PollResult<ssize_t> poll_write(const char* buffer, size_t size, Waker&& waker) = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCWRITE_HPP
