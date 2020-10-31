#include "OutgoingResponse.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageReader.hpp"
#include "Header.hpp"
#include "IncomingRequest.hpp"
#include "MessageReader.hpp"
#include "RfcConstants.hpp"
#include "Status.hpp"
#include "StringReader.hpp"

using std::move;

namespace http {

auto OutgoingResponseBuilder::status(Status status) -> OutgoingResponseBuilder&
{
    _status = status;
    return *this;
}

auto OutgoingResponseBuilder::header(HeaderName name, const HeaderValue& value) -> OutgoingResponseBuilder&
{
    _message.header(move(name), string(value));
    return *this;
}

auto OutgoingResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body) -> OutgoingResponseBuilder&
{

    _message.body(OutgoingBody(move(body)));
    header(http::header::TRANSFER_ENCODING, "chunked");
    return *this;
}

auto OutgoingResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> OutgoingResponseBuilder&
{
    _message.body(OutgoingBody(move(body)));
    return header(http::header::CONTENT_LENGTH, std::to_string(content_length));
}

auto OutgoingResponseBuilder::build() && -> OutgoingResponse
{
    if (!_status.has_value()) {
        throw std::runtime_error("ResponseBuilder: no status set");
    }
    return OutgoingResponse(
        _version,
        *_status,
        move(_message).build());
}

OutgoingResponseBuilder::OutgoingResponseBuilder()
    : _status(http::status::IM_A_TEAPOT)
{
}

auto OutgoingResponseBuilder::version(Version version) -> OutgoingResponseBuilder&
{
    _version = version;
    return *this;
}

auto OutgoingResponseBuilder::cgi(Cgi&& proc) -> OutgoingResponseBuilder&
{
    _message.body(OutgoingBody(move(proc)));
    return *this;
}

auto OutgoingResponseBuilder::headers(Headers&& headers) -> OutgoingResponseBuilder&
{
    _message.headers(move(headers));
    return *this;
}

OutgoingResponse::OutgoingResponse(Version version, Status status, OutgoingMessage&& message)
    : _version(version)
    , _status(status)
    , _message(move(message))
{
}

auto OutgoingResponse::get_headers() const -> Headers const&
{
    return _message.get_headers();
}

auto OutgoingResponse::get_version() const -> Version const&
{
    return _version;
}

auto OutgoingResponse::get_status() const -> Status const&
{
    return _status;
}

auto OutgoingResponse::get_body() const -> optional<OutgoingBody> const&
{
    return _message.get_body();
}

auto OutgoingResponse::get_body() -> optional<OutgoingBody>&
{
    return _message.get_body();
}

auto OutgoingResponse::get_header(HeaderName const& needle_name) const -> optional<HeaderValue>
{
    auto res = _message.get_header(needle_name);
    if (res.has_value()) {
        return string(*res);
    } else {
        return option::nullopt;
    }
}

auto OutgoingResponse::get_message() -> OutgoingMessage&
{
    return _message;
}

auto OutgoingResponse::get_message() const -> OutgoingMessage const&
{
    return _message;
}

ResponseReader::ResponseReader(OutgoingResponse& response)
    : _response(response)
{
    auto& body = _response.get_body();
    if (body.has_value() && body->is_cgi()) {
        // CGI has been set - switch to "wait for CGI ready" status;
        _state = CheckCgiState;
    } else {
        // CGI has not been set - begin writing a body
        new (&_status_line) StatusLineReader(_response.get_version(), _response.get_status());
        _state = StatusLineReaderState;
    }
}

auto ResponseReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case CheckCgiState: {
            if (_response.get_body()->get_cgi().poll_success(Waker(waker))) {
                // Change to Status state
                _state = StatusLineReaderState;
                new (&_status_line) StatusLineReader(_response.get_version(), _response.get_status());
                return poll_read(buffer, move(waker));
            }
            return PollResult<IoResult>::pending();
        } break;
        case StatusLineReaderState: {
            auto poll_res = _status_line.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _status_line.~StatusLineReader();
                    // Change to next state
                    if (_response.get_body().has_value() && _response.get_body()->is_cgi()) {
                        _state = CgiState;
                        new (&_cgi_reader) CgiReader(_response.get_body()->get_cgi());
                    } else {
                        _state = MessageState;
                        new (&_message) MessageReader(_response.get_message());
                    }
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case MessageState: {
            return _message.poll_read(buffer, move(waker));
        } break;
        case CgiState: {
            return _cgi_reader.poll_read(buffer, move(waker));
        } break;
    }
}

ResponseReader::~ResponseReader()
{
    switch (_state) {
        case StatusLineReaderState: {
            _status_line.~StatusLineReader();
        } break;
        case MessageState: {
            _message.~MessageReader();
        }
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
        case StatusLineReaderState: {
            new (&_status_line) StatusLineReader(move(other._status_line));
        } break;
        case MessageState: {
            new (&_message) MessageReader(move(other._message));
        } break;
        case CgiState: {
            new (&_cgi_reader) CgiReader(move(other._cgi_reader));
        } break;
    }
}

ResponseReader::StatusLineReader::StatusLineReader(Version const& version, Status const& status)
    : _version(version)
    , _status(status)
{
    // start out with Version
    _current = _version.version_string;
}

//
// Poller for reading the request-line
//
auto ResponseReader::StatusLineReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
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
                auto b = utils::uint64_to_string(_status.code);
                _status_code_buf[0] = b[0];
                _status_code_buf[1] = b[1];
                _status_code_buf[2] = b[2];
                _current = std::string_view(_status_code_buf, 3);
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

ResponseReader::CgiReader::CgiReader(Cgi& cgi)
    : _cgi(cgi)
{
}

auto ResponseReader::CgiReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
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

}
