//
// Created by boat on 9/30/20.
//

#ifndef WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
#define WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP

#include "../futures/IFuture.hpp"
#include "../futures/Waker.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"

using futures::IFuture;
using futures::PollResult;
using futures::Waker;

namespace ioruntime {

class IoCopyFuture: public IFuture<void> {
public:
    IoCopyFuture() = delete;
    IoCopyFuture(IAsyncRead& reader, IAsyncWrite& writer);
    auto poll(Waker&& waker) -> PollResult<void> override;
private:
    uint8_t buffer[8192];
    size_t head = 0;
    enum State {
        Reading,
        Writing
    } state = Reading;
    IAsyncRead& reader;
    IAsyncWrite& writer;
};

}

#endif //WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
