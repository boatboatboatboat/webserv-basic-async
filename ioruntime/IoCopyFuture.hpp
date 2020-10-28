//
// Created by boat on 9/30/20.
//

#ifndef WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
#define WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP

#include "../futures/IFuture.hpp"
#include "../futures/Waker.hpp"
#include "../option/optional.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"

using futures::IFuture;
using futures::PollResult;
using futures::Waker;
using option::optional;

namespace ioruntime {

template <typename R = void, typename W = void>
class IoCopyFuture : public IFuture<void> {
public:
    IoCopyFuture() = delete;
    explicit IoCopyFuture(R&& reader, W&& writer);
    auto poll(Waker&& waker) -> PollResult<void> override;
    [[nodiscard]] auto get_bytes_written() const -> size_t;
    [[nodiscard]] auto has_written_bytes() const -> bool;
    IoCopyFuture(IoCopyFuture&& other) noexcept
        : _bytes_written(other._bytes_written)
        , _owned_reader(std::move(other._owned_reader))
        , _owned_writer(std::move(other._owned_writer))
        , _reader(_owned_reader)
        , _writer(_owned_writer)
    {
        if (other._span.has_value()) {
            memcpy(_buffer, other._buffer, other._span->size());
            _span = span(_buffer, other._span->size());
        }
    }
    // c++ has an issue where you can't reassign a reference
    // so we can't move-assign this
    IoCopyFuture operator=(IoCopyFuture&&) = delete;

private:
    uint8_t _buffer[4096];
    optional<span<uint8_t>> _span;
    size_t _bytes_written = 0;
    size_t _head = 0;
    bool _read_finished = false;
    enum State {
        Reading,
        Writing
    } state
        = Reading;
    R _owned_reader;
    W _owned_writer;
    IAsyncRead& _reader;
    IAsyncWrite& _writer;
};

template <typename R>
class IoCopyFuture<R, void> : public IFuture<void> {
public:
    IoCopyFuture() = delete;
    explicit IoCopyFuture(R&& reader, IAsyncWrite& writer);
    auto poll(Waker&& waker) -> PollResult<void> override;
    [[nodiscard]] auto get_bytes_written() const -> size_t;
    [[nodiscard]] auto has_written_bytes() const -> bool;
    IoCopyFuture(IoCopyFuture&& other) noexcept
        : _bytes_written(other._bytes_written)
        , _owned_reader(std::move(other._owned_reader))
        , _reader(_owned_reader)
        , _writer(other._writer)
    {
        if (other._span.has_value()) {
            memcpy(_buffer, other._buffer, other._span->size());
            _span = span(_buffer, other._span->size());
        }
    }
    // c++ has an issue where you can't reassign a reference
    // so we can't move-assign this
    IoCopyFuture operator=(IoCopyFuture&&) = delete;

private:
    uint8_t _buffer[4096];
    optional<span<uint8_t>> _span;
    size_t _bytes_written = 0;
    size_t _head = 0;
    bool _read_finished = false;
    enum State {
        Reading,
        Writing
    } state
        = Reading;
    R _owned_reader;
    IAsyncRead& _reader;
    IAsyncWrite& _writer;
};

template <typename W>
class IoCopyFuture<void, W> : public IFuture<void> {
public:
    IoCopyFuture() = delete;
    explicit IoCopyFuture(IAsyncRead& reader, W&& writer);
    auto poll(Waker&& waker) -> PollResult<void> override;
    [[nodiscard]] auto get_bytes_written() const -> size_t;
    [[nodiscard]] auto has_written_bytes() const -> bool;
    IoCopyFuture(IoCopyFuture&& other) noexcept
        : _owned_writer(std::move(other._owned_writer))
        , _bytes_written(other._bytes_written)
        , _reader(other._character_stream)
        , _writer(_owned_writer)
    {
        if (other._span.has_value()) {
            memcpy(_buffer, other._buffer, other._span->size());
            _span = span(_buffer, other._span->size());
        }
    }
    // c++ has an issue where you can't reassign a reference
    // so we can't move-assign this
    IoCopyFuture operator=(IoCopyFuture&&) = delete;

private:
    uint8_t _buffer[4096];
    optional<span<uint8_t>> _span;
    size_t _bytes_written = 0;
    size_t _head = 0;
    bool _read_finished = 0;
    enum State {
        Reading,
        Writing
    } state
        = Reading;
    W _owned_writer;
    IAsyncRead& _reader;
    IAsyncWrite& _writer;
};

template <>
class IoCopyFuture<void, void> : public IFuture<void> {
public:
    IoCopyFuture() = delete;
    explicit IoCopyFuture(IAsyncRead& reader, IAsyncWrite& writer)
        : _reader(reader)
        , _writer(writer)
    {
    }
    auto poll(Waker&& waker) -> PollResult<void> override
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
    [[nodiscard]] auto get_bytes_written() const -> size_t
    {
        return _bytes_written;
    }
    [[nodiscard]] auto has_written_bytes() const -> bool
    {
        return _bytes_written != 0;
    };

    IoCopyFuture(IoCopyFuture&& other) noexcept
        : _bytes_written(other._bytes_written)
        , _reader(other._reader)
        , _writer(other._writer)
    {
        if (other._span.has_value()) {
            memcpy(_buffer, other._buffer, other._span->size());
            _span = span(_buffer, other._span->size());
        }
    }
    IoCopyFuture operator=(IoCopyFuture&&) = delete;
private:
    uint8_t _buffer[4096];
    optional<span<uint8_t>> _span;
    size_t _bytes_written = 0;
    size_t _head = 0;
    bool _read_finished = false;
    enum State {
        Reading,
        Writing
    } state
        = Reading;
    IAsyncRead& _reader;
    IAsyncWrite& _writer;
};

using RefIoCopyFuture = IoCopyFuture<void, void>;

}

#include "IoCopyFuture.ipp"

#endif //WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
