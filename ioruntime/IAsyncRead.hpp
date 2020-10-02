//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_IASYNCREAD_HPP
#define WEBSERV_IORUNTIME_IASYNCREAD_HPP

#include "../futures/PollResult.hpp"
#include "../utils/span.hpp"
#include "IoResult.hpp"
#include <algorithm>

using futures::PollResult;
using utils::span;

namespace ioruntime {

class IAsyncRead {
public:
    virtual auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> = 0;
    virtual ~IAsyncRead() = 0;
};

}

#endif //WEBSERV_IORUNTIME_IASYNCREAD_HPP
