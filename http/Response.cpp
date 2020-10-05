#include "Response.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "Header.hpp"
#include "Request.hpp"
#include "RfcConstants.hpp"
#include "Status.hpp"
#include "StringReader.hpp"

using std::move;

namespace http {

auto ResponseBuilder::status(Status status) -> ResponseBuilder&
{
    _status = status;
    return *this;
}

auto ResponseBuilder::header(HeaderName name, const HeaderValue& value) -> ResponseBuilder&
{
    if (!_headers.has_value()) {
        _headers = Headers();
    }
    _headers->push_back(Header { move(name), value });
    return *this;
}

auto ResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body) -> ResponseBuilder&
{
    _body = Body(move(body));
    header(http::header::TRANSFER_ENCODING, "chunked");
    return *this;
}

auto ResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> ResponseBuilder&
{
    _body = Body(move(body));
    return header(http::header::CONTENT_LENGTH, std::to_string(content_length));
}

auto ResponseBuilder::build() -> Response
{
    if (!_status.has_value()) {
        throw std::runtime_error("ResponseBuilder: no status set");
    }
    if (!_body.has_value() && _cgi.has_value()) {
        _body = Body(move(*_cgi));
    }
    return Response(
        move(_headers),
        _version,
        *_status,
        move(_body));
}

ResponseBuilder::ResponseBuilder()
    : _status(http::status::IM_A_TEAPOT)
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

Response::Response(optional<Headers>&& headers, Version version, Status status, optional<Body>&& body)
    : _version(version)
    , _status(status)
{
    if (headers.has_value()) {
        _headers = move(*headers);
    }
    if (body.has_value()) {
        _body = move(*body);
    }
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

auto Response::get_body() const -> optional<Body> const&
{
    return _body;
}

auto Response::get_body() -> optional<Body>&
{
    return _body;
}

auto Response::get_header(HeaderName const& needle_name) const -> optional<HeaderValue>
{
    for (auto& header : _headers) {
        if (utils::str_eq_case_insensitive(header.name, needle_name)) {
            return header.value;
        }
    }
    return option::nullopt;
}

ResponseReader::ResponseReader(Response& response)
    : _response(response)
{
    auto& body = _response.get_body();
    if (body.has_value() && body->is_cgi()) {
        // CGI has been set - switch to "wait for CGI ready" status;
        _state = CheckCgiState;
    } else {
        // CGI has not been set - begin writing a body
        new (&_request_line) RequestLineReader(_response.get_version(), _response.get_status());
        _state = RequestLineState;
    }
}

auto ResponseReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case CheckCgiState: {
            if (_response.get_body()->get_cgi().poll_success(Waker(waker))) {
                // Change to Status state
                _state = RequestLineState;
                new (&_request_line) RequestLineReader(_response.get_version(), _response.get_status());
                return poll_read(buffer, move(waker));
            }
            return PollResult<IoResult>::pending();
        } break;
        case RequestLineState: {
            auto poll_res = _request_line.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _request_line.~RequestLineReader();
                    // Change to next state
                    auto& body = _response.get_body();
                    if (body.has_value() && body->is_cgi()) {
                        // CGI writes its own headers, so skip to the CGI state
                        _state = CgiBodyState;
                        new (&_cgi_body) CgiBodyReader(body->get_cgi());
                    } else {
                        // No CGI - write our own headers
                        _state = HeaderState;
                        // FIXME: Headers == None is not handled
                        new (&_header) HeadersReader(_response.get_headers());
                    }
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case HeaderState: {
            auto poll_res = _header.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _header.~HeadersReader();
                    new (&_header_terminator) HeaderTerminatorReader();
                    _state = HeaderTerminatorState;
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case HeaderTerminatorState: {
            auto poll_res = _header_terminator.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _header_terminator.~HeaderTerminatorReader();
                    // Change to Body state
                    if (!_response.get_body().has_value()) {
                        // We don't have a body - just terminate
                        return PollResult<IoResult>::ready(IoResult::eof());
                    } else if (_response.get_header(http::header::CONTENT_LENGTH).has_value()) {
                        // Content-Length means regular body
                        _state = BodyState;
                        new (&_body) BodyReader(*_response.get_body());
                    } else {
                        // No content-length means chunked transfer
                        _state = ChunkedBodyState;
                        new (&_chunked_body) ChunkedBodyReader(*_response.get_body());
                    }
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case BodyState: {
            return _body.poll_read(buffer, move(waker));
        } break;
        case ChunkedBodyState: {
            return _chunked_body.poll_read(buffer, move(waker));
        } break;
        case CgiBodyState: {
            return _cgi_body.poll_read(buffer, move(waker));
        } break;
    }
}

ResponseReader::~ResponseReader()
{
    switch (_state) {
        case RequestLineState: {
            _request_line.~RequestLineReader();
        } break;
        case HeaderState: {
            _header.~HeadersReader();
        } break;
        case HeaderTerminatorState: {
            _header_terminator.~HeaderTerminatorReader();
        } break;
        case BodyState: {
            _body.~BodyReader();
        } break;
        case ChunkedBodyState: {
            _chunked_body.~ChunkedBodyReader();
        } break;
        case CgiBodyState: {
            _cgi_body.~CgiBodyReader();
        } break;
        default:
            break;
    }
}

ResponseReader::ResponseReader(ResponseReader&& other) noexcept
    : _response(other._response)
    , _state(other._state)
{
    switch (_state) {
        case CheckCgiState: {
            // do nothing
        } break;
        case RequestLineState: {
            new (&_request_line) RequestLineReader(move(other._request_line));
        } break;
        case HeaderState: {
            new (&_header) HeadersReader(move(other._header));
        } break;
        case HeaderTerminatorState: {
            new (&_header_terminator) HeaderTerminatorReader();
        } break;
        case BodyState: {
            new (&_body) BodyReader(move(other._body));
        } break;
        case ChunkedBodyState: {
            new (&_chunked_body) ChunkedBodyReader(move(other._chunked_body));
        } break;
        case CgiBodyState: {
            new (&_cgi_body) CgiBodyReader(move(other._cgi_body));
        } break;
    }
}

ResponseReader::RequestLineReader::RequestLineReader(Version const& version, Status const& status)
    : _version(version)
    , _status(status)
{
    // start out with Version
    _current = _version.version_string;
}

//
// Poller for reading the request-line
//
auto ResponseReader::RequestLineReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
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

ResponseReader::HeadersReader::HeadersReader(Headers const& headers)
    : _headers(headers)
{
    _header_it = _headers.begin();
    if (_header_it != _headers.end()) {
        _current = _header_it->name;
    } else {
        _state = Clrf;
        _current = "";
        --_header_it; // lmao
    }
}

auto ResponseReader::HeadersReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case Name: {
            if (_current.empty()) {
                _state = Splitter;
                _current = ": ";
            }
        } break;
        case Splitter: {
            if (_current.empty()) {
                _state = Value;
                _current = _header_it->value;
            }
        } break;
        case Value: {
            if (_current.empty()) {
                _state = Clrf;
                _current = CLRF;
            }
        } break;
        case Clrf: {
            if (_current.empty()) {
                // current depleted, select next header
                ++_header_it;
                if (_header_it == _headers.end()) {
                    return PollResult<IoResult>::ready(IoResult::eof());
                }
                _current = _header_it->name;
                _state = Name;
            }
        } break;
    }
    auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
    memcpy(buffer.data(), _current.data(), written);
    _current.remove_prefix(written);
    waker();
    return PollResult<IoResult>::ready(written);
}

ResponseReader::BodyReader::BodyReader(Body& body)
    : _body(body)
{
}

auto ResponseReader::BodyReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    return _body.poll_read(buffer, move(waker));
}

ResponseReader::ChunkedBodyReader::ChunkedBodyReader(Body& body)
    : _state(Clrf2)
    , _current(_data_buf, 0)
    , _current_body(_data_buf, 0)
    , _body(body)
{
}

auto ResponseReader::ChunkedBodyReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case ChunkSize: {
            // write the chunk size
            if (_current.empty()) {
                _current = span((uint8_t*)CLRF.data(), CLRF.size());
                _state = Clrf1;
                return poll_read(buffer, move(waker));
            }
        } break;
        case Clrf1: {
            // write the CLRF after the chunk size
            if (_current.empty()) {
                _current = _current_body;
                _state = ChunkData;
                return poll_read(buffer, move(waker));
            }
        } break;
        case ChunkData: {
            // write the actual chunk data
            if (_current.empty()) {
                _current = span((uint8_t*)CLRF.data(), CLRF.size());
                _state = Clrf2;
                return poll_read(buffer, move(waker));
            }
        } break;
        case Clrf2: {
            // write the CLRF after the chunk data
            if (_current.empty()) {
                auto poll_res = _body.poll_read(span(_data_buf, sizeof(_data_buf)), Waker(waker));
                if (poll_res.is_ready()) {
                    auto res = poll_res.get();
                    if (res.is_eof()) {
                        _current = span((uint8_t*)"0\r\n\r\n", 5);
                        _state = Eof;
                    } else if (res.is_error()) {
                        return PollResult<IoResult>::ready(IoResult::error());
                    } else if (res.is_success()) {
                        _current_body = span(_data_buf, res.get_bytes_read());
                        // FIXME: sprintf hex
                        std::sprintf(_num, "%zx", res.get_bytes_read());
                        _current = span((uint8_t*)_num, utils::strlen(_num));
                        _state = ChunkSize;
                    }
                    return poll_read(buffer, move(waker));
                } else {
                    return PollResult<IoResult>::pending();
                }
            }
        } break;
        case Eof: {
            // write 0\r\n\r\n
            if (_current.empty()) {
                return PollResult<IoResult>::ready(IoResult::eof());
            }
        } break;
    }
    auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
    memcpy(buffer.data(), _current.data(), written);
    _current.remove_prefix_inplace(written);
    waker();
    return PollResult<IoResult>::ready(written);
}

ResponseReader::CgiBodyReader::CgiBodyReader(Cgi& cgi)
    : _cgi(cgi)
{
}

auto ResponseReader::CgiBodyReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case WriteRequestBody: {
            if (!writer_finished) {
                auto poll_res = _cgi.poll_write_body(Waker(waker));
                if (poll_res.is_ready()) {
                    auto res = poll_res.get();
                    if (res.is_eof()) {
                        writer_finished = true;
                        _state = ReadCgi;
                        return poll_read(buffer, move(waker));
                    }
                } else if (poll_res.is_pending()) {
                    _state = ReadCgi;
                    return poll_read(buffer, move(waker));
                }
            } else {
                _state = ReadCgi;
                return poll_read(buffer, move(waker));
            }
            return PollResult<IoResult>::pending();
        } break;
        case ReadCgi: {
            auto poll_res = _cgi.poll_read(buffer, move(waker));
            if (!writer_finished) {
                if (poll_res.is_pending()) {
                    _state = WriteRequestBody;
                }
            }
            return poll_res;
        } break;
    }
}

auto ResponseReader::HeaderTerminatorReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
    memcpy(buffer.data(), _current.data(), written);
    _current.remove_prefix(written);
    waker();
    return PollResult<IoResult>::ready(written);
}

}
