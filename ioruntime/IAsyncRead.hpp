//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_IASYNCREAD_HPP
#define WEBSERV_IORUNTIME_IASYNCREAD_HPP

#include <algorithm>
#include "../futures/PollResult.hpp"

using futures::PollResult;

namespace ioruntime {

class IAsyncRead {
public:
    virtual PollResult<ssize_t> poll_read(char* buffer, size_t size, Waker&& waker) = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCREAD_HPP
