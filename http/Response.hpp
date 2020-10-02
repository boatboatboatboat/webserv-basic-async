//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IoCopyFuture.hpp"
#include "../net/Socket.hpp"
#include "Body.hpp"
#include "DefaultPageBody.hpp"
#include "Header.hpp"
#include "Request.hpp"
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

class Response;

class ResponseBuilder final {
public:
    ResponseBuilder();
    auto version(Version version) -> ResponseBuilder&;
    auto status(Status status) -> ResponseBuilder&;
    auto header(HeaderName name, const HeaderValue& value) -> ResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body) -> ResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> ResponseBuilder&;
    auto cgi(Cgi&& proc) -> ResponseBuilder&;
    auto build() -> Response;

private:
    optional<Headers> _headers;
    optional<Version> _version = version::v1_1;
    optional<Status> _status;
    optional<Body> _body;
    optional<Cgi> _cgi = option::nullopt;
};

class Response final {
public:
    // ctors/dtors
    Response() = delete;
    Response(Headers&& headers, Version&& version, Status&& status, Body&& body, optional<Cgi>&& cgi);
    Response(Response&&);
    auto operator=(Response&&) -> Response&;
    ~Response() = default;

    // getter methods
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(HeaderName const& needle_name) const -> optional<HeaderValue>;
    [[nodiscard]] auto get_version() const -> Version const&;
    [[nodiscard]] auto get_status() const -> Status const&;
    [[nodiscard]] auto get_body() const -> Body const&;
    [[nodiscard]] auto get_cgi() const -> optional<Cgi> const&;
    [[nodiscard]] auto get_cgi() -> optional<Cgi>&;

private:
    // FIXME: Proxy-based requests don't have a 'set' header like this
    Headers _headers;
    Version _version;
    Status _status;
    Body _body;
    optional<Cgi> _cgi;
};

class ResponseReader final : public IAsyncRead {
public:
    explicit ResponseReader(Response& response);
    ~ResponseReader() override;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // inner readers
    class StatusReader : public IAsyncRead {
    public:
        explicit StatusReader(Version const& version, Status const& status);
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
    class HeaderReader : public IAsyncRead {
    public:
        explicit HeaderReader(Header const& header);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            Name,
            Splitter,
            Value,
            Clrf
        } _state
            = Name;
        string_view _current;
        Header const& _header;
    };
    class BodyReader : public IAsyncRead {
    public:
        explicit BodyReader(Response const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        Response const& _response;
    };
    class ChunkedBodyReader : public IAsyncRead {
    public:
        explicit ChunkedBodyReader(Response const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        Response const& _response;
    };
    class CgiBodyReader : public IAsyncRead {
    public:
        explicit CgiBodyReader(Response const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        Response const& _response;
    };

    // Members
    Response& _response;
    enum State {
        CheckCgiSuccess,
        StatusState,
        Header,
        Body,
        ChunkedBody,
        CgiBody
    } _state;
    union {
        StatusReader _status;
        HeaderReader _header;
        BodyReader _body;
        ChunkedBodyReader _chunked_body;
        CgiBodyReader _cgi_body;
    };
};

/*
class LegacyHttpResponse {
public:
    LegacyHttpResponse();
    explicit LegacyHttpResponse(
        std::map<HeaderName, HeaderValue>&& response_headers,
        Status status,
        BoxPtr<ioruntime::IAsyncRead>&& response_body,
        optional<Cgi>&& _cgi);
    LegacyHttpResponse(LegacyHttpResponse&& other) noexcept;

    auto operator=(LegacyHttpResponse&& other) noexcept -> LegacyHttpResponse&;
    virtual ~LegacyHttpResponse();
    auto poll_respond(net::Socket& socket, Waker&& waker) -> PollResult<void>;
    auto write_response(net::Socket& socket, Waker&& waker) -> bool;

private:
    enum State {
        // Status line states
        WriteStatusVersion,
        WriteStatusSpace1,
        WriteStatusCode,
        WriteStatusSpace2,
        WriteStatusMessage,
        WriteStatusCRLF,

        // Header line states
        WriteHeaderName,
        WriteHeaderSplit,
        WriteHeaderValue,
        WriteHeaderCRLF,
        WriteSeperatorCLRF,

        // Regular body states
        ReadBody,
        WriteBody,

        // Chunked body states
        WriteChunkedBodySize,
        WriteChunkedBodyCLRF1,
        WriteChunkedBody,
        WriteChunkedBodyCLRF2,
        WriteChunkedBodyEof,

        // Cgi body states
        WriteToCgiBody,
        ReadCgiBody,
        // WriteBody,
    };
    char buf[8192] {};
    char num[get_max_num_size(sizeof(buf))] {};
    State state { WriteStatusVersion };
    std::string_view current;
    std::string_view body_view;
    ssize_t written { 0 };
    std::map<HeaderName, HeaderValue> response_headers;
    std::map<HeaderName, HeaderValue>::const_iterator header_it;
    Status response_status;
    Version response_version;
    BoxPtr<ioruntime::IAsyncRead> response_body;
    optional<Cgi> _cgi;
    optional<ioruntime::IoCopyFuture> _icf;
};
*/

}

#endif //WEBSERV_HTTPRESPONSE_HPP
