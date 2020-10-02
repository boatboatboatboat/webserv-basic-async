//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_IASYNCWRITE_HPP
#define WEBSERV_IORUNTIME_IASYNCWRITE_HPP

#include "../futures/PollResult.hpp"
#include "../utils/span.hpp"
#include "IoResult.hpp"
#include <algorithm>

using futures::PollResult;
using utils::span;

namespace ioruntime {

class IAsyncWrite {
public:
    virtual auto poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> = 0;
    virtual ~IAsyncWrite() = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCWRITE_HPP
