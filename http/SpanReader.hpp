//
// Created by boat on 9/30/20.
//

#ifndef WEBSERV_HTTP_SPANREADER_HPP
#define WEBSERV_HTTP_SPANREADER_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/span.hpp"

using utils::span;

namespace ioruntime {

class SpanReader : public IAsyncRead {
public:
    explicit SpanReader(span<uint8_t> inner);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
    [[nodiscard]] auto get_bytes_read() const -> size_t;
    void discard_first(size_t discardable);
private:
    size_t _bytes_read;
    span<uint8_t> _inner;
};

template <typename T>
class OwningSpanReader : public IAsyncRead {
public:
    explicit OwningSpanReader(T&& inner);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
private:
    SpanReader _real_reader;
    T _t;
};

// TODO: split to ipp
template <typename T>
OwningSpanReader<T>::OwningSpanReader(T&& inner)
    : _t(std::move(inner))
{
    _real_reader = span(inner.data(), inner.size());
}

template <typename T>
auto OwningSpanReader<T>::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    return _real_reader.poll_read(buffer, std::move(waker));
}

}

#endif //WEBSERV_HTTP_SPANREADER_HPP
