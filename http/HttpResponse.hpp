//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IoCopyFuture.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
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

// using HttpBody = BoxPtr<ioruntime::IAsyncRead>;

class HttpBody final : public IAsyncRead {
public:
    // special methods
    HttpBody() = delete;
    HttpBody(HttpBody&&);
    HttpBody& operator=(HttpBody&&);
    ~HttpBody();

    // ctors
    explicit HttpBody(BoxPtr<ioruntime::IAsyncRead>&& reader);
    explicit HttpBody(Cgi&& cgi);

    // methods
    [[nodiscard]] auto is_reader() const -> bool;
    [[nodiscard]] auto is_cgi() const -> bool;
    auto get_cgi() -> Cgi&;

    // implement: IAsyncRead
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<ioruntime::IoResult> override;

private:
    enum Tag {
        ReaderTag,
        CgiTag,
    } tag;
    union {
        BoxPtr<ioruntime::IAsyncRead> _reader;
        Cgi _cgi;
    };
};

class LegacyHttpResponse;

class HttpResponseBuilder final {
public:
    HttpResponseBuilder();
    auto version(HttpVersion version) -> HttpResponseBuilder&;
    auto status(HttpStatus status) -> HttpResponseBuilder&;
    auto header(HttpHeaderName name, const HttpHeaderValue& value) -> HttpResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body) -> HttpResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> HttpResponseBuilder&;
    auto cgi(Cgi&& proc) -> HttpResponseBuilder&;
    auto build() -> LegacyHttpResponse;

private:
    HttpHeaders _headers;
    HttpVersion _version = version::v1_1;
    HttpStatus _status;
    HttpBody _body;
    optional<Cgi> _cgi = option::nullopt;
};

class HttpResponse final {
public:
    // ctors/dtors
    HttpResponse() = delete;
    HttpResponse(HttpHeaders&& headers, HttpVersion&& version, HttpStatus&& status, HttpBody&& body, optional<Cgi>&& cgi);
    HttpResponse(HttpResponse&&);
    auto operator=(HttpResponse&&) -> HttpResponse&;
    ~HttpResponse() = default;

    // getter methods
    [[nodiscard]] auto get_headers() const -> HttpHeaders const&;
    [[nodiscard]] auto get_header(HttpHeaderName const& needle_name) const -> optional<HttpHeaderValue>;
    [[nodiscard]] auto get_version() const -> HttpVersion const&;
    [[nodiscard]] auto get_status() const -> HttpStatus const&;
    [[nodiscard]] auto get_body() const -> HttpBody const&;
    [[nodiscard]] auto get_cgi() const -> optional<Cgi> const&;
    [[nodiscard]] auto get_cgi() -> optional<Cgi>&;

private:
    // FIXME: Proxy-based requests don't have a 'set' header like this
    HttpHeaders _headers;
    HttpVersion _version;
    HttpStatus _status;
    HttpBody _body;
    optional<Cgi> _cgi;
};

class HttpResponseReader : public IAsyncRead {
public:
    explicit HttpResponseReader(HttpResponse& response);
    ~HttpResponseReader() override;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // inner readers
    class StatusReader : public IAsyncRead {
    public:
        explicit StatusReader(HttpResponse const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            Version,
            Space1,
            Code,
            Space2,
            Message,
            Clrf,
        } _state
            = Version;
        char _status_code_buf[3];
        string_view _current;
        HttpResponse const& _response;
    };
    class HeaderReader : public IAsyncRead {
    public:
        explicit HeaderReader(HttpResponse const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        HttpResponse const& _response;
    };
    class BodyReader : public IAsyncRead {
    public:
        explicit BodyReader(HttpResponse const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        HttpResponse const& _response;
    };
    class ChunkedBodyReader : public IAsyncRead {
    public:
        explicit ChunkedBodyReader(HttpResponse const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        HttpResponse const& _response;
    };
    class CgiBodyReader : public IAsyncRead {
    public:
        explicit CgiBodyReader(HttpResponse const& response);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        HttpResponse const& _response;
    };

    // Members
    HttpResponse& _response;
    enum State {
        CheckCgiSuccess,
        Status,
        Header,
        Body,
        ChunkedBody,
        CgiBody
    } state;
    union {
        StatusReader _status;
        HeaderReader _header;
        BodyReader _body;
        ChunkedBodyReader _chunked_body;
        CgiBodyReader _cgi_body;
    };
};

class LegacyHttpResponse {
public:
    LegacyHttpResponse();
    explicit LegacyHttpResponse(
        std::map<HttpHeaderName, HttpHeaderValue>&& response_headers,
        HttpStatus status,
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
    std::map<HttpHeaderName, HttpHeaderValue> response_headers;
    std::map<HttpHeaderName, HttpHeaderValue>::const_iterator header_it;
    HttpStatus response_status;
    HttpVersion response_version;
    BoxPtr<ioruntime::IAsyncRead> response_body;
    optional<Cgi> _cgi;
    optional<ioruntime::IoCopyFuture> _icf;
};

}

#endif //WEBSERV_HTTPRESPONSE_HPP
