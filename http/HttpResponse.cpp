#include "HttpResponse.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "StringBody.hpp"

namespace http {

HttpResponse::~HttpResponse() = default;

HttpResponse::HttpResponse()
    : response_headers()
    , response_status(HTTP_STATUS_IM_A_TEAPOT)
    , response_version(HTTP_VERSION_1_1)
    , response_body(nullptr)
{
}

HttpResponse::HttpResponse(std::map<HttpHeaderName, HttpHeaderValue>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body, bool cgi_mode)
    : response_headers(std::move(response_headers))
    , response_status(status)
    , response_version(HTTP_VERSION_1_1)
    , response_body(std::move(response_body))
    , cgi_mode(cgi_mode)
{
    if (this->response_body.get() == nullptr) {
        // fixme: _body is BoxPtr which is allocated, memory alloc error can't recover properly
        this->response_body = BoxPtr<DefaultPageBody>::make(status);
        this->response_headers.insert(std::make_pair(http::header::TRANSFER_ENCODING, "chunked"));
    }
}

auto HttpResponse::write_response(net::Socket& socket, Waker&& waker) -> bool
{
    auto str = current.data() + written;
    auto len = current.length() - written;

    auto poll_result = socket.poll_write(str, len, std::move(waker));

    if (poll_result.is_ready()) {
        auto result = poll_result.get();

        if (result < 0) {
            throw std::runtime_error("HttpResponse: write_respond: status write error");
        }
        written += result;

        if (written == (ssize_t)current.length()) {
            written = 0;
            return true;
        }
    }
    return false;
}

auto HttpResponse::poll_respond(net::Socket& socket, Waker&& waker) -> PollResult<void>
{
    TRACEPRINT("responder state: " << state);
    switch (state) {
    case WriteStatusVersion: {
        current = this->response_version.version_string;
    } break;
    case WriteStatusSpace1:
    case WriteStatusSpace2: {
        current = HTTP_SP;
    } break;
    case WriteStatusCode: {
        std::sprintf(buf, "%u", this->response_status.code);
        current = buf;
    } break;
    case WriteStatusCRLF: {
        current = HTTP_CRLF;
        header_it = response_headers.begin();
    } break;
    case WriteStatusMessage: {
        current = this->response_status.message;
    } break;
    case WriteHeaderName: {
        if (cgi_mode) {
            state = ReadBody;
            return poll_respond(socket, std::move(waker));
        }
        if (header_it == response_headers.end()) {
            state = WriteSeperatorCLRF;
            return poll_respond(socket, std::move(waker));
        }
        current = header_it->first;
    } break;
    case WriteHeaderSplit: {
        current = ": ";
    } break;
    case WriteHeaderValue: {
        current = header_it->second;
    } break;
    case WriteSeperatorCLRF:
    case WriteHeaderCRLF: {
        current = HTTP_CRLF;
    } break;
    case ReadBody: {
        auto is_chunked = false;
        // if response_headers.contains(http::header::TRANSFER_ENCODING)
        if (!cgi_mode) {
            if (std::any_of(response_headers.begin(), response_headers.end(), [](auto& i) { return i.first == http::header::TRANSFER_ENCODING; })) {
                // FIXME: bad chunked check
                if (response_headers[http::header::TRANSFER_ENCODING].find("chunked") != std::string::npos) {
                    is_chunked = true;
                }
            }
        }

        auto poll_result = response_body->poll_read(buf, sizeof(buf), Waker(waker));
        if (poll_result.is_pending())
            return PollResult<void>::pending();
        if (poll_result.get() == 0) {
            if (is_chunked) {
                current = "0\r\n\r\n";
                state = WriteChunkedBodyEof;
            } else {
                TRACEPRINT("end of body");
                return PollResult<void>::ready();
            }
        }
        body_view = std::string_view(buf, poll_result.get());
        current = body_view;
        if (is_chunked) {
            state = WriteChunkedBodySize;
            // FIXME: sprintf
            std::sprintf(num, "%zx", body_view.length());
        } else {
            state = WriteBody;
        }
        return poll_respond(socket, std::move(waker));
    } break;
    case WriteBody: {
        // current is already set in previous step
    } break;
    case WriteChunkedBodySize: {
        current = num;
    } break;
    case WriteChunkedBodyCLRF1: {
        current = HTTP_CRLF;
    } break;
    case WriteChunkedBody: {
        current = body_view;
    } break;
    case WriteChunkedBodyCLRF2: {
        current = HTTP_CRLF;
    } break;
    case WriteChunkedBodyEof: {
        // do nothing
    } break;
    }
    if (write_response(socket, Waker(waker))) {
        if (state == WriteChunkedBodyEof) {
            return PollResult<void>::ready();
        }
        if (state != WriteHeaderCRLF && state != WriteBody && state != WriteChunkedBodyCLRF2) {
            state = static_cast<State>(static_cast<int>(state) + 1);
        } else if (state == WriteBody || state == WriteChunkedBodyCLRF2) {
            state = ReadBody;
        } else if (state == WriteHeaderCRLF) {
            ++header_it;
            state = WriteHeaderName;
        }
        return poll_respond(socket, std::move(waker));
    }
    return PollResult<void>::pending();
}

HttpResponse::HttpResponse(HttpResponse&& other) noexcept
    : state(other.state)
    , current(other.current)
    , response_headers(std::move(other.response_headers))
    , header_it(other.header_it)
    , response_status(other.response_status)
    , response_version(other.response_version)
    , response_body(std::move(other.response_body))
    , cgi_mode(other.cgi_mode)
{
}

auto HttpResponse::operator=(HttpResponse&& other) noexcept -> HttpResponse&
{
    if (&other == this) {
        return *this;
    }

    this->state = other.state;
    this->current = other.current;
    this->response_headers = std::move(other.response_headers);
    this->header_it = other.header_it;
    this->response_status = other.response_status;
    this->response_body = std::move(other.response_body);
    this->response_version = other.response_version;
    this->cgi_mode = other.cgi_mode;
    return *this;
}

auto HttpResponseBuilder::status(HttpStatus status) -> HttpResponseBuilder&
{
    _status = status;
    return *this;
}

auto HttpResponseBuilder::header(HttpHeaderName name, const HttpHeaderValue& value) -> HttpResponseBuilder&
{
    _headers.insert(std::pair<HttpHeaderName, HttpHeaderValue>(name, value));
    return *this;
}

auto HttpResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body) -> HttpResponseBuilder&
{
    _body = std::move(body);
    header(http::header::TRANSFER_ENCODING, "chunked");
    return *this;
}

auto HttpResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> HttpResponseBuilder&
{
    _body = std::move(body);
    return header(http::header::CONTENT_LENGTH, std::to_string(content_length));
}

auto HttpResponseBuilder::build() -> HttpResponse
{
    return HttpResponse(std::move(_headers), _status, std::move(_body), _cgi_mode);
}

HttpResponseBuilder::HttpResponseBuilder()
    : _status(HTTP_STATUS_IM_A_TEAPOT)
    , _body(nullptr)
{
}

auto HttpResponseBuilder::version(HttpVersion version) -> HttpResponseBuilder&
{
    _version = version;
    return *this;
}

auto HttpResponseBuilder::cgi() -> HttpResponseBuilder&
{
    _cgi_mode = true;
    return *this;
}

}