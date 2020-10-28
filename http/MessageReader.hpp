//
// Created by boat on 10/24/20.
//

#ifndef WEBSERV_HTTP_MESSAGEREADER_HPP
#define WEBSERV_HTTP_MESSAGEREADER_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "OutgoingMessage.hpp"

namespace http {

namespace inner_readers {

    // Get the max size of the buffer to store a chunked transfer size in
    static inline constexpr auto get_max_num_size(size_t s) -> size_t
    {
        // gets the length of a number in hex notation, + 1
        size_t n = 0;
        do {
            n += 1;
            s /= 16;
        } while (s);
        return n + 1;
    }

    // reads all the headers + a clrf
    class HeadersReader final : public IAsyncRead {
    public:
        HeadersReader() = delete;
        explicit HeadersReader(Headers const& headers);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            Name,
            Splitter,
            Value,
            Clrf,
        } _state
            = Name;
        string_view _current;
        Headers::const_iterator _header_it;
        Headers const& _headers;
    };
    // reads the body 'raw'
    class BodyReader final : public IAsyncRead {
    public:
        explicit BodyReader(OutgoingBody& body);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        OutgoingBody& _body;
    };
    // reads the body, serialized in cte format
    class ChunkedBodyReader final : public IAsyncRead {
    public:
        explicit ChunkedBodyReader(OutgoingBody& body);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            ChunkSize,
            Clrf1,
            ChunkData,
            Clrf2,
            Eof,
        } _state;
        uint8_t _data_buf[512];
        char _num[get_max_num_size(sizeof(_data_buf))];
        span<uint8_t> _current;
        span<uint8_t> _current_body;
        OutgoingBody& _body;
    };
    class HeaderTerminatorReader final : public IAsyncRead {
    public:
        HeaderTerminatorReader() = default;
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        string_view _current = http::CLRF;
    };

}

class MessageReader final : public ioruntime::IAsyncRead {
public:
    MessageReader() = delete;
    explicit MessageReader(OutgoingMessage& message);
    MessageReader(MessageReader&&) noexcept;
    auto operator=(MessageReader&&) noexcept -> MessageReader& = delete;
    MessageReader(MessageReader const&) = delete;
    auto operator=(MessageReader const&) -> MessageReader& = delete;
    ~MessageReader() override;

    auto poll_read(span<uint8_t>, Waker&&) -> PollResult<IoResult> override;

private:
    OutgoingMessage& _message;

    enum State {
        HeaderState,
        HeaderTerminatorState,
        BodyState,
        ChunkedBodyState,
    } _state;
    union {
        inner_readers::HeadersReader _header;
        inner_readers::HeaderTerminatorReader _header_terminator;
        inner_readers::BodyReader _body;
        inner_readers::ChunkedBodyReader _chunked_body;
    };
};

}

#endif //WEBSERV_HTTP_MESSAGEREADER_HPP
