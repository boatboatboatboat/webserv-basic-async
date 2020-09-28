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
    : _response_headers()
    , _response_status(HTTP_STATUS_IM_A_TEAPOT)
    , _response_version(HTTP_VERSION_1_1)
    , _response_body(nullptr)
{
}

HttpResponse::HttpResponse(std::map<HttpHeaderName, HttpHeaderValue>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body)
    : _response_headers(std::move(response_headers))
    , _response_status(status)
    , _response_version(HTTP_VERSION_1_1)
    , _response_body(std::move(response_body))
{
    if (this->_response_body.get() == nullptr) {
        // fixme: _body is BoxPtr which is allocated, memory alloc error can't recover properly
        this->_response_body = BoxPtr<DefaultPageBody>::make(status);
        this->_response_headers.insert(std::make_pair(http::header::TRANSFER_ENCODING, "chunked"));
    }
}

auto HttpResponse::write_response(net::Socket& socket, Waker&& waker) -> bool
{
    auto str = _current.data() + written;
    auto len = _current.length() - written;

    auto poll_result = socket.poll_write(str, len, std::move(waker));

    if (poll_result.is_ready()) {
        auto result = poll_result.get();

        if (result < 0) {
            throw std::runtime_error("HttpResponse: write_respond: status write error");
        }
        written += result;

        if (written == (ssize_t)_current.length()) {
            written = 0;
            return true;
        }
    }
    return false;
}

auto HttpResponse::poll_respond(net::Socket& socket, Waker&& waker) -> PollResult<void>
{
    TRACEPRINT("responder state: " << _state);
    switch (_state) {
    case WriteStatusVersion: {
        _current = this->_response_version.version_string;
    } break;
    case WriteStatusSpace1:
    case WriteStatusSpace2: {
        _current = HTTP_SP;
    } break;
    case WriteStatusCode: {
        std::sprintf(buf, "%u", this->_response_status.code);
        _current = buf;
    } break;
    case WriteStatusCRLF: {
        _current = HTTP_CRLF;
        _header_it = _response_headers.begin();
    } break;
    case WriteStatusMessage: {
        _current = this->_response_status.message;
    } break;
    case WriteHeaderName: {
        if (_header_it == _response_headers.end()) {
            _state = WriteSeperatorCLRF;
            return poll_respond(socket, std::move(waker));
        }
        _current = _header_it->first;
    } break;
    case WriteHeaderSplit: {
        _current = ": ";
    } break;
    case WriteHeaderValue: {
        _current = _header_it->second;
    } break;
    case WriteSeperatorCLRF:
    case WriteHeaderCRLF: {
        _current = HTTP_CRLF;
    } break;
    case ReadBody: {
        auto is_chunked = false;
        // if response_headers.contains(http::header::TRANSFER_ENCODING)
        if (std::any_of(_response_headers.begin(), _response_headers.end(), [](auto& i) { return i.first == http::header::TRANSFER_ENCODING; })) {
            // FIXME: bad chunked check
            if (_response_headers[http::header::TRANSFER_ENCODING].find("chunked") != std::string::npos) {
                is_chunked = true;
            }
        }

        auto poll_result = _response_body->poll_read(buf, sizeof(buf), Waker(waker));
        if (poll_result.is_pending())
            return PollResult<void>::pending();
        if (poll_result.get() == 0) {
            if (is_chunked) {
                _current = "0\r\n\r\n";
                _state = WriteChunkedBodyEof;
            } else {
                TRACEPRINT("end of body");
                return PollResult<void>::ready();
            }
        }
        body_view = std::string_view(buf, poll_result.get());
        _current = body_view;
        if (is_chunked) {
            _state = WriteChunkedBodySize;
            // FIXME: sprintf
            std::sprintf(num, "%zx", body_view.length());
        } else {
            _state = WriteBody;
        }
        return poll_respond(socket, std::move(waker));
    } break;
    case WriteBody: {
        // current is already set in previous step
    } break;
    case WriteChunkedBodySize: {
        _current = num;
    } break;
    case WriteChunkedBodyCLRF1: {
        _current = HTTP_CRLF;
    } break;
    case WriteChunkedBody: {
        _current = body_view;
    } break;
    case WriteChunkedBodyCLRF2: {
        _current = HTTP_CRLF;
    } break;
    case WriteChunkedBodyEof: {
        // do nothing
    } break;
    }
    if (write_response(socket, Waker(waker))) {
        if (_state == WriteChunkedBodyEof) {
            return PollResult<void>::ready();
        }
        if (_state != WriteHeaderCRLF && _state != WriteBody && _state != WriteChunkedBodyCLRF2) {
            _state = static_cast<State>(static_cast<int>(_state) + 1);
        } else if (_state == WriteBody || _state == WriteChunkedBodyCLRF2) {
            _state = ReadBody;
        } else if (_state == WriteHeaderCRLF) {
            ++_header_it;
            _state = WriteHeaderName;
        }
        return poll_respond(socket, std::move(waker));
    }
    return PollResult<void>::pending();
}

HttpResponse::HttpResponse(HttpResponse&& other) noexcept
    : _state(other._state)
    , _current(other._current)
    , _response_headers(std::move(other._response_headers))
    , _header_it(other._header_it)
    , _response_status(other._response_status)
    , _response_version(other._response_version)
    , _response_body(std::move(other._response_body))
{
}

auto HttpResponse::operator=(HttpResponse&& other) noexcept -> HttpResponse&
{
    if (&other == this) {
        return *this;
    }

    _state = other._state;
    _current = other._current;
    _response_headers = std::move(other._response_headers);
    _header_it = other._header_it;
    _response_status = other._response_status;
    _response_body = std::move(other._response_body);
    _response_version = other._response_version;
    return *this;
}

auto HttpResponseBuilder::status(HttpStatus status) -> HttpResponseBuilder&
{
    _status = status;
    return *this;
}

auto HttpResponseBuilder::header(HttpHeaderName name, const HttpHeaderValue& value) -> HttpResponseBuilder&
{
    _headers.emplace(name, std::string(value));
    return *this;
}

auto HttpResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body) -> HttpResponseBuilder&
{
    // No content-length is set - use chunked transfer encoding
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
    return HttpResponse(std::move(_headers), _status, std::move(_body));
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

}