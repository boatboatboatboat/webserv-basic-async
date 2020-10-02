//
// Created by boat on 9/30/20.
//

#ifndef WEBSERV_HTTP_SPANBODY_HPP
#define WEBSERV_HTTP_SPANBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/span.hpp"

using utils::span;

namespace ioruntime {

class SpanBody: public IAsyncRead {
public:
    explicit SpanBody(span<uint8_t> inner);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
private:
    span<uint8_t> _inner;
};

}

#endif //WEBSERV_HTTP_SPANBODY_HPP
