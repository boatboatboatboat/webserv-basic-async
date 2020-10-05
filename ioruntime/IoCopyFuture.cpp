//
// Created by boat on 9/30/20.
//

#include "IoCopyFuture.hpp"

namespace ioruntime {

auto IoCopyFuture::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
    case Reading: {
        auto poll_res = reader.poll_read(span(buffer, sizeof(buffer)), Waker(waker));

        if (poll_res.is_ready()) {
            auto res = poll_res.get();
            if (res.is_error()) {
                throw std::runtime_error("CopyFuture: read error");
            } else if (res.is_eof()) {
                return PollResult<void>::ready();
            } else {
                state = Writing;
                _span = span(buffer, res.get_bytes_read());
                return poll(std::move(waker));
            }
        }
    } break;
    case Writing: {
        // fixme: bad size (expected readres, found buffersiz)
        auto poll_res = writer.poll_write(*_span, Waker(waker));

        if (poll_res.is_ready()) {
            auto res = poll_res.get();
            if (res.is_error()) {
                throw std::runtime_error("CopyFuture: write error");
            } else if (res.is_eof()) {
                return PollResult<void>::ready();
            } else {
                _span->remove_prefix_inplace(res.get_bytes_read());
                if (_span->empty()) {
                    state = Reading;
                    return poll(std::move(waker));
                }
            }
        }
    } break;
    }
    return PollResult<void>::pending();
}

IoCopyFuture::IoCopyFuture(ioruntime::IAsyncRead& reader, ioruntime::IAsyncWrite& writer)
    : reader(reader), writer(writer)
{
}

}