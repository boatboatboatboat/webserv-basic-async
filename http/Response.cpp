#include "Response.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "Header.hpp"
#include "Request.hpp"
#include "RfcConstants.hpp"
#include "Status.hpp"
#include "StringBody.hpp"

using std::move;

namespace http {

/*
LegacyHttpResponse::~LegacyHttpResponse() = default;

LegacyHttpResponse::LegacyHttpResponse()
    : response_headers()
    , response_status(http::status::IM_A_TEAPOT)
    , response_version(http::version::v1_1)
    , response_body(nullptr)
{
}

LegacyHttpResponse::LegacyHttpResponse(std::map<HeaderName, HeaderValue>&& response_headers, Status status, BoxPtr<ioruntime::IAsyncRead>&& response_body, optional<Cgi>&& _proc)
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
*/

auto ResponseBuilder::status(Status status) -> ResponseBuilder&
{
    _status = status;
    return *this;
}

auto ResponseBuilder::header(HeaderName name, const HeaderValue& value) -> ResponseBuilder&
{
    _headers.push_back(Header { move(name), value });
    return *this;
}

auto ResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body) -> ResponseBuilder&
{
    _body = std::move(body);
    header(http::header::TRANSFER_ENCODING, "chunked");
    return *this;
}

auto ResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> ResponseBuilder&
{
    _body = std::move(body);
    return header(http::header::CONTENT_LENGTH, std::to_string(content_length));
}

auto ResponseBuilder::build() -> Response
{
    return Response(
        move(*_headers),
        move(*_version),
        move(*_status),
        move(*_body),
        move(*_cgi));
}

ResponseBuilder::ResponseBuilder()
    : _status(http::status::IM_A_TEAPOT)
    , _body(nullptr)
{
}

auto ResponseBuilder::version(Version version) -> ResponseBuilder&
{
    _version = version;
    return *this;
}

auto ResponseBuilder::cgi(Cgi&& proc) -> ResponseBuilder&
{
    _cgi = std::move(proc);
    return *this;
}

Response::Response(Headers&& headers, Version&& version, Status&& status, Body&& body, optional<Cgi>&& cgi)
    : _headers(move(headers))
    , _version(version)
    , _status(status)
    , _body(move(body))
    , _cgi(move(cgi))
{
}

auto Response::get_headers() const -> Headers const&
{
    return _headers;
}

auto Response::get_version() const -> Version const&
{
    return _version;
}

auto Response::get_status() const -> Status const&
{
    return _status;
}

auto Response::get_body() const -> Body const&
{
    return _body;
}

auto Response::get_cgi() const -> optional<Cgi> const&
{
    return _cgi;
}

auto Response::get_cgi() -> optional<Cgi>&
{
    return _cgi;
}

auto Response::get_header(HeaderName const& needle_name) const -> optional<HeaderValue>
{
    for (auto& header : _headers) {
        if (utils::str_eq_case_insensitive(header.name, needle_name)) {
            return header.value;
        }
    }
}

ResponseReader::ResponseReader(Response& response)
    : _response(response)
{
    if (response.get_cgi().has_value()) {
        // CGI has been set - switch to "wait for CGI ready" status;
        _state = CheckCgiSuccess;
    } else {
        // CGI has not been set - begin writing a body
        _state = Status;
    }
}

auto ResponseReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
    case CheckCgiSuccess: {
        if (_response.get_cgi()->poll_success(Waker(waker))) {
            // Change to Status state
            _state = Status;
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
                    _state = CgiBody;
                    new (&_cgi_body) CgiBodyReader(_response);
                } else {
                    // No CGI - write our own headers
                    _state = Header;
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
                    _state = Body;
                    new (&_body) BodyReader(_response);
                } else {
                    // No content-length means chunked transfer
                    _state = ChunkedBody;
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

ResponseReader::~ResponseReader()
{
    switch (_state) {
    case StatusState: {
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

ResponseReader::StatusReader::StatusReader(Version const& version, Status const& status)
    : _version(version)
    , _status(status)
{
    // start out with Version
    _current = version.version_string;
}

auto ResponseReader::StatusReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
    case VersionState: {
        if (_current.empty()) {
            _current = http::SP;
            _state = Space1;
        }
    } break;
    case Space1: {
        if (_current.empty()) {
            std::sprintf(_status_code_buf, "%u", _status.code);
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
            _current = _status.message;
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
    auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
    memcpy(buffer.data(), _current.data(), written);
    _current.remove_prefix(written);
    buffer.remove_prefix_inplace(written);
    waker();
    return PollResult<IoResult>::ready(written);
}

}