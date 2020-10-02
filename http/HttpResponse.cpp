#include "HttpResponse.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "StringBody.hpp"

using std::move;

namespace http {

LegacyHttpResponse::~LegacyHttpResponse() = default;

LegacyHttpResponse::LegacyHttpResponse()
    : response_headers()
    , response_status(http::status::IM_A_TEAPOT)
    , response_version(http::version::v1_1)
    , response_body(nullptr)
{
}

LegacyHttpResponse::LegacyHttpResponse(std::map<HttpHeaderName, HttpHeaderValue>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body, optional<Cgi>&& _proc)
    : response_headers(std::move(response_headers))
    , response_status(status)
    , response_version(http::version::v1_1)
    , response_body(std::move(response_body))
    , _cgi(std::move(_proc))
{
    if (this->response_body.get() == nullptr) {
        // fixme: _body is BoxPtr which is allocated, memory alloc error can't recover properly
        this->response_body = BoxPtr<DefaultPageBody>::make(status);
        this->response_headers.insert(std::make_pair(http::header::TRANSFER_ENCODING, "chunked"));
    }
}

auto LegacyHttpResponse::write_response(net::Socket& socket, Waker&& waker) -> bool
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

auto LegacyHttpResponse::poll_respond(net::Socket& socket, Waker&& waker) -> PollResult<void>
{
    TRACEPRINT("responder state: " << state);
    switch (state) {
    case WriteStatusVersion: {
        current = this->response_version.version_string;
    } break;
    case WriteStatusSpace1:
    case WriteStatusSpace2: {
        current = SP;
    } break;
    case WriteStatusCode: {
        std::sprintf(buf, "%u", this->response_status.code);
        current = buf;
    } break;
    case WriteStatusCRLF: {
        current = CLRF;
        header_it = response_headers.begin();
    } break;
    case WriteStatusMessage: {
        current = this->response_status.message;
    } break;
    case WriteHeaderName: {
        if (_cgi.has_value()) {
            state = ReadCgiBody;
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
        current = CLRF;
    } break;
    case ReadBody: {
        auto is_chunked = false;
        if (!_cgi.has_value()) {
            // if response_headers.contains(http::header::TRANSFER_ENCODING)
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
    case ReadCgiBody: {
        auto poll_result = _cgi->poll_read(buf, sizeof(buf), Waker(waker));
        if (poll_result.is_pending()) {
            return PollResult<void>::pending();
        }
        if (poll_result.get() == 0) {
            return PollResult<void>::ready();
        }
        body_view = string_view(buf, poll_result.get());
        current = body_view;
        state = WriteBody;
    } break;
    case WriteToCgiBody: {
        auto poll_result = _icf->poll(Waker(waker));
        if (poll_result.is_ready()) {
            state = ReadCgiBody;
        }
    } break;
    case WriteBody: {
        // current is already set in previous step
    } break;
    case WriteChunkedBodySize: {
        current = num;
    } break;
    case WriteChunkedBodyCLRF1: {
        current = CLRF;
    } break;
    case WriteChunkedBody: {
        current = body_view;
    } break;
    case WriteChunkedBodyCLRF2: {
        current = CLRF;
    } break;
    case WriteChunkedBodyEof: {
        // do nothing
    } break;
    }
    if (write_response(socket, Waker(waker))) {
        if (state == WriteChunkedBodyEof) {
            return PollResult<void>::ready();
        }
        if (
            state != WriteHeaderCRLF
            && state != WriteBody
            && state != WriteChunkedBodyCLRF2
            && state != WriteToCgiBody
            && state != ReadCgiBody) {
            state = static_cast<State>(static_cast<int>(state) + 1);
        } else if (state == WriteBody || state == WriteChunkedBodyCLRF2) {
            if (_cgi.has_value()) {
                state = ReadCgiBody;
            } else {
                state = ReadBody;
            }
        } else if (state == WriteHeaderCRLF) {
            ++header_it;
            state = WriteHeaderName;
        }
        return poll_respond(socket, std::move(waker));
    }
    return PollResult<void>::pending();
}

LegacyHttpResponse::LegacyHttpResponse(LegacyHttpResponse&& other) noexcept
    : state(other.state)
    , current(other.current)
    , response_headers(std::move(other.response_headers))
    , header_it(other.header_it)
    , response_status(other.response_status)
    , response_version(other.response_version)
    , response_body(std::move(other.response_body))
    , _cgi(std::move(other._cgi))
{
}

auto LegacyHttpResponse::operator=(LegacyHttpResponse&& other) noexcept -> LegacyHttpResponse&
{
    if (&other == this) {
        return *this;
    }

    state = other.state;
    current = other.current;
    response_headers = std::move(other.response_headers);
    header_it = other.header_it;
    response_status = other.response_status;
    response_body = std::move(other.response_body);
    response_version = other.response_version;
    _cgi = std::move(other._cgi);
    return *this;
}

auto HttpResponseBuilder::status(HttpStatus status) -> HttpResponseBuilder&
{
    _status = status;
    return *this;
}

auto HttpResponseBuilder::header(HttpHeaderName name, const HttpHeaderValue& value) -> HttpResponseBuilder&
{
    _headers.push_back(HttpHeader { move(name), value });
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

auto HttpResponseBuilder::build() -> LegacyHttpResponse
{
    return LegacyHttpResponse(std::move(_headers), _status, std::move(_body), std::move(_cgi));
}

HttpResponseBuilder::HttpResponseBuilder()
    : _status(http::status::IM_A_TEAPOT)
    , _body(nullptr)
{
}

auto HttpResponseBuilder::version(HttpVersion version) -> HttpResponseBuilder&
{
    _version = version;
    return *this;
}

auto HttpResponseBuilder::cgi(Cgi&& proc) -> HttpResponseBuilder&
{
    _cgi = std::move(proc);
    return *this;
}

HttpResponse::HttpResponse(HttpHeaders&& headers, HttpVersion&& version, HttpStatus&& status, HttpBody&& body, optional<Cgi>&& cgi)
    : _headers(move(headers))
    , _version(version)
    , _status(status)
    , _body(move(body))
    , _cgi(move(cgi))
{
}

auto HttpResponse::get_headers() const -> HttpHeaders const&
{
    return _headers;
}

auto HttpResponse::get_version() const -> HttpVersion const&
{
    return _version;
}

auto HttpResponse::get_status() const -> HttpStatus const&
{
    return _status;
}

auto HttpResponse::get_body() const -> HttpBody const&
{
    return _body;
}

auto HttpResponse::get_cgi() const -> optional<Cgi> const&
{
    return _cgi;
}

auto HttpResponse::get_cgi() -> optional<Cgi>&
{
    return _cgi;
}

auto HttpResponse::get_header(HttpHeaderName const& needle_name) const -> optional<HttpHeaderValue>
{
    for (auto& header : _headers) {
        if (utils::str_eq_case_insensitive(header.name, needle_name)) {
            return header.value;
        }
    }
}

HttpResponseReader::HttpResponseReader(HttpResponse& response)
    : _response(response)
{
    if (response.get_cgi().has_value()) {
        // CGI has been set - switch to "wait for CGI ready" status;
        state = CheckCgiSuccess;
    } else {
        // CGI has not been set - begin writing a body
        state = Status;
    }
}

auto HttpResponseReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (state) {
    case CheckCgiSuccess: {
        if (_response.get_cgi()->poll_success(Waker(waker))) {
            // Change to Status state
            state = Status;
            new (&_status) StatusReader(_response);
            return poll_read(buffer, move(waker));
        }
        return PollResult<IoResult>::pending();
    } break;
    case Status: {
        auto poll_res = _status.poll_read(buffer, Waker(waker));
        if (poll_res.is_ready()) {
            if (poll_res.get().is_eof()) {
                // Clean up old state
                _status.~StatusReader();
                // Change to next state
                if (_response.get_cgi().has_value()) {
                    // CGI writes its own headers, so skip to the CGI state
                    state = CgiBody;
                    new (&_cgi_body) CgiBodyReader(_response);
                } else {
                    // No CGI - write our own headers
                    state = Header;
                    new (&_header) HeaderReader(_response);
                }
                return PollResult<IoResult>::pending();
            }
        }
        return poll_res;
    } break;
    case Header: {
        auto poll_res = _header.poll_read(buffer, Waker(waker));
        if (poll_res.is_ready()) {
            if (poll_res.get().is_eof()) {
                // Clean up old state
                _header.~HeaderReader();
                // Change to Body state
                if (_response.get_header(http::header::CONTENT_LENGTH).has_value()) {
                    // Content-Length means regular body
                    state = Body;
                    new (&_body) BodyReader(_response);
                } else {
                    // No content-length means chunked transfer
                    state = ChunkedBody;
                    new (&_chunked_body) ChunkedBodyReader(_response);
                }
                return PollResult<IoResult>::pending();
            }
        }
        return poll_res;
    } break;
    case Body: {
        return _body.poll_read(buffer, move(waker));
    } break;
    case ChunkedBody: {
        return _chunked_body.poll_read(buffer, move(waker));
    } break;
    case CgiBody: {
        return _cgi_body.poll_read(buffer, move(waker));
    } break;
    }
}

HttpResponseReader::~HttpResponseReader()
{
    switch (state) {
    case Status: {
        _status.~StatusReader();
    } break;
    case Header: {
        _header.~HeaderReader();
    } break;
    case Body: {
        _body.~BodyReader();
    } break;
    case ChunkedBody: {
        _chunked_body.~ChunkedBodyReader();
    } break;
    case CgiBody: {
        _cgi_body.~CgiBodyReader();
    } break;
    default:
        break;
    }
}

HttpResponseReader::StatusReader::StatusReader(const HttpResponse& response)
    : _response(response)
{
    // start out with Version
    _current = _response.get_version().version_string;
}

auto HttpResponseReader::StatusReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
    case Version: {
        if (_current.empty()) {
            _current = http::SP;
            _state = Space1;
        }
    } break;
    case Space1: {
        if (_current.empty()) {
            std::sprintf(_status_code_buf, "%u", _response.get_status().code);
            _current = _status_code_buf;
            _state = Code;
        }
    } break;
    case Code: {
        if (_current.empty()) {
            _current = http::SP;
            _state = Space2;
        }
    } break;
    case Space2: {
        if (_current.empty()) {
            _current = _response.get_status().message;
            _state = Message;
        }
    } break;
    case Message: {
        if (_current.empty()) {
            _current = http::CLRF;
            _state = Clrf;
        }
    } break;
    case Clrf: {
        if (_current.empty()) {
            return PollResult<IoResult>::ready(IoResult::eof());
        }
    } break;
    }
}



}