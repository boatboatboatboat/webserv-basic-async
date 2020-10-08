//
// Created by boat on 9/30/20.
//

#include "IoCopyFuture.hpp"

namespace ioruntime {

auto IoCopyFuture::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
        case Reading: {
            auto poll_res = _reader.poll_read(span(_buffer, sizeof(_buffer)), Waker(waker));

            if (poll_res.is_ready()) {
                auto res = poll_res.get();
                if (res.is_error()) {
                    throw std::runtime_error("CopyFuture: read error");
                } else if (res.is_eof()) {
                    return PollResult<void>::ready();
                } else {
                    state = Writing;
                    _span = span(_buffer, res.get_bytes_read());
                    return poll(std::move(waker));
                }
            }
        } break;
        case Writing: {
            auto poll_res = _writer.poll_write(*_span, Waker(waker));

            if (poll_res.is_ready()) {
                auto res = poll_res.get();
                if (res.is_error()) {
                    throw std::runtime_error("CopyFuture: write error");
                } else if (res.is_eof()) {
                    return PollResult<void>::ready();
                } else {
                    _bytes_written += res.get_bytes_read();
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
    : _reader(reader)
    , _writer(writer)
{
}

auto IoCopyFuture::get_bytes_written() const -> size_t
{
    return _bytes_written;
}

auto IoCopyFuture::has_written_bytes() const -> bool
{
    return _bytes_written != 0;
}

}