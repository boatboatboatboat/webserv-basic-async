//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IoCopyFuture.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageReader.hpp"
#include "Header.hpp"
#include "IncomingRequest.hpp"
#include "OutgoingBody.hpp"
#include "RfcConstants.hpp"
#include "Status.hpp"
#include "Version.hpp"
#include <map>
#include <string>

using cgi::Cgi;

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

namespace http {

class OutgoingResponse;

class OutgoingResponseBuilder final {
public:
    OutgoingResponseBuilder();
    auto version(Version version) -> OutgoingResponseBuilder&;
    auto status(Status status) -> OutgoingResponseBuilder&;
    auto header(HeaderName name, const HeaderValue& value) -> OutgoingResponseBuilder&;
    auto headers(Headers&& headers) -> OutgoingResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body) -> OutgoingResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> OutgoingResponseBuilder&;
    auto cgi(Cgi&& proc) -> OutgoingResponseBuilder&;
    auto build() -> OutgoingResponse;

private:
    optional<Headers> _headers;
    Version _version = version::v1_1;
    optional<Status> _status;
    optional<OutgoingBody> _body;
    optional<Cgi> _cgi = option::nullopt;
};

class OutgoingResponse final {
public:
    // ctors/dtors
    OutgoingResponse() = delete;
    OutgoingResponse(optional<Headers>&& headers, Version version, Status status, optional<OutgoingBody>&& body);
    ~OutgoingResponse() = default;
    OutgoingResponse(OutgoingResponse&&) noexcept = default;
    auto operator=(OutgoingResponse&&) noexcept -> OutgoingResponse& = default;

    // getter methods
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(HeaderName const& needle_name) const -> optional<HeaderValue>;
    [[nodiscard]] auto get_version() const -> Version const&;
    [[nodiscard]] auto get_status() const -> Status const&;
    [[nodiscard]] auto get_body() const -> optional<OutgoingBody> const&;
    [[nodiscard]] auto get_body() -> optional<OutgoingBody>&;

    void drop_body();

private:
    // FIXME: Proxy-based requests don't have a 'set' header like this
    Headers _headers;
    Version _version;
    Status _status;
    optional<OutgoingBody> _body;
};

class ResponseReader final : public IAsyncRead {
public:
    ResponseReader() = delete;
    ResponseReader(ResponseReader&&) noexcept;
    auto operator=(ResponseReader&&) noexcept -> ResponseReader&;
    explicit ResponseReader(OutgoingResponse& response);
    ~ResponseReader() override;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // inner readers

    // reads the request line
    class RequestLineReader final : public IAsyncRead {
    public:
        RequestLineReader() = delete;
        explicit RequestLineReader(Version const& version, Status const& status);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            VersionState,
            Space1,
            Code,
            Space2,
            Message,
            Clrf,
        } _state
            = VersionState;
        char _status_code_buf[3];
        string_view _current;
        Version const& _version;
        Status const& _status;
    };
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
    // reads the body from cgi
    class CgiBodyReader final : public IAsyncRead {
    public:
        CgiBodyReader() = delete;
        explicit CgiBodyReader(Cgi& cgi);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            WriteRequestBody,
            ReadCgi,
        } _state
            = WriteRequestBody;
        bool writer_finished = false;
        Cgi& _cgi;
    };

    class HeaderTerminatorReader final : public IAsyncRead {
    public:
        HeaderTerminatorReader() = default;
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        string_view _current = http::CLRF;
    };

    // Members
    OutgoingResponse& _response;
    enum State {
        CheckCgiState,
        RequestLineState,
        HeaderState,
        HeaderTerminatorState,
        BodyState,
        ChunkedBodyState,
        CgiBodyState
    } _state;
    union {
        RequestLineReader _request_line;
        HeadersReader _header;
        HeaderTerminatorReader _header_terminator;
        BodyReader _body;
        ChunkedBodyReader _chunked_body;
        CgiBodyReader _cgi_body;
    };
};

}

#endif //WEBSERV_HTTPRESPONSE_HPP
