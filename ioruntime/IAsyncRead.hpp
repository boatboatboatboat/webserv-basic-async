//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_IASYNCREAD_HPP
#define WEBSERV_IORUNTIME_IASYNCREAD_HPP

#include "../futures/PollResult.hpp"
#include <algorithm>

using futures::PollResult;

namespace ioruntime {

class IAsyncRead {
public:
    virtual auto poll_read(char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> = 0;
    virtual ~IAsyncRead() = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCREAD_HPP
