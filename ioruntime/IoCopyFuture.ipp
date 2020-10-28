//
// Created by boat on 9/30/20.
//

#include "IoCopyFuture.hpp"

namespace ioruntime {

template <typename R, typename W>
auto IoCopyFuture<R, W>::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
        case Reading: {
            for (;;) {
                auto poll_res = _reader.poll_read(span(_buffer + _head, sizeof(_buffer) - _head), Waker(waker));

                if (poll_res.is_ready()) {
                    auto res = poll_res.get();
                    if (res.is_error()) {
                        throw std::runtime_error("CopyFuture: read error");
                    } else if (res.is_eof()) {
                        _read_finished = true;
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    } else {
                        _head += res.get_bytes_read();
                    }
                    if (_head == sizeof(_buffer)) {
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    }
                } else {
                    state = Writing;
                    _span = span(_buffer, _head);
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
                } else {
                    _bytes_written += res.get_bytes_read();
                    _span->remove_prefix_inplace(res.get_bytes_read());
                    if (_span->empty()) {
                        if (_read_finished) {
                            return PollResult<void>::ready();
                        }
                        state = Reading;
                        _head = 0;
                        return poll(std::move(waker));
                    }
                }
            }
        } break;
    }
    return PollResult<void>::pending();
}

template <typename R>
auto IoCopyFuture<R, void>::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
        case Reading: {
            for (;;) {
                auto poll_res = _reader.poll_read(span(_buffer + _head, sizeof(_buffer) - _head), Waker(waker));

                if (poll_res.is_ready()) {
                    auto res = poll_res.get();
                    if (res.is_error()) {
                        throw std::runtime_error("CopyFuture: read error");
                    } else if (res.is_eof()) {
                        _read_finished = true;
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    } else {
                        _head += res.get_bytes_read();
                    }
                    if (_head == sizeof(_buffer)) {
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    }
                } else {
                    state = Writing;
                    _span = span(_buffer, _head);
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
                } else {
                    _bytes_written += res.get_bytes_read();
                    _span->remove_prefix_inplace(res.get_bytes_read());
                    if (_span->empty()) {
                        if (_read_finished) {
                            return PollResult<void>::ready();
                        }
                        state = Reading;
                        _head = 0;
                        return poll(std::move(waker));
                    }
                }
            }
        } break;
    }
    return PollResult<void>::pending();
}

template <typename W>
auto IoCopyFuture<void, W>::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
        case Reading: {
            for (;;) {
                auto poll_res = _reader.poll_read(span(_buffer + _head, sizeof(_buffer) - _head), Waker(waker));

                if (poll_res.is_ready()) {
                    auto res = poll_res.get();
                    if (res.is_error()) {
                        throw std::runtime_error("CopyFuture: read error");
                    } else if (res.is_eof()) {
                        _read_finished = true;
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    } else {
                        _head += res.get_bytes_read();
                    }
                    if (_head == sizeof(_buffer)) {
                        state = Writing;
                        _span = span(_buffer, _head);
                        return poll(std::move(waker));
                    }
                } else {
                    state = Writing;
                    _span = span(_buffer, _head);
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
                } else {
                    _bytes_written += res.get_bytes_read();
                    _span->remove_prefix_inplace(res.get_bytes_read());
                    if (_span->empty()) {
                        if (_read_finished) {
                            DBGPRINT("ReadFinished");
                            return PollResult<void>::ready();
                        }
                        state = Reading;
                        _head = 0;
                        return poll(std::move(waker));
                    }
                }
            }
        } break;
    }
    return PollResult<void>::pending();
}

template <typename R, typename W>
IoCopyFuture<R, W>::IoCopyFuture(R&& reader, W&& writer)
    : _owned_reader(std::forward<R>(reader))
    , _owned_writer(std::forward<W>(writer))
    , _reader(_owned_reader)
    , _writer(_owned_writer)
{
}

template <typename R>
IoCopyFuture<R, void>::IoCopyFuture(R&& reader, IAsyncWrite& writer)
    : _owned_reader(std::forward<R>(reader))
    , _reader(_owned_reader)
    , _writer(writer)
{
}

template <typename W>
IoCopyFuture<void, W>::IoCopyFuture(IAsyncRead& reader, W&& writer)
    : _owned_writer(std::forward<W>(writer))
    , _reader(reader)
    , _writer(_owned_writer)
{
}

template <typename R, typename W>
auto IoCopyFuture<R, W>::get_bytes_written() const -> size_t
{
    return _bytes_written;
}

template <typename R>
auto IoCopyFuture<R, void>::get_bytes_written() const -> size_t
{
    return _bytes_written;
}

template <typename W>
auto IoCopyFuture<void, W>::get_bytes_written() const -> size_t
{
    return _bytes_written;
}

template <typename R, typename W>
auto IoCopyFuture<R, W>::has_written_bytes() const -> bool
{
    return _bytes_written != 0;
}

}