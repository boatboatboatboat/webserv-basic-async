//
// Created by boat on 10/1/20.
//

#include "Server.hpp"
#include "../fs/File.hpp"

#include <utility>

namespace {
using ioruntime::GlobalRuntime;
using std::move;
}

namespace http {

TimeoutError::TimeoutError()
    : std::runtime_error("Timed out")
{
}

ServerBuilder::ServerBuilder(net::IpAddress ip, uint16_t port)
    : _ip(ip)
    , _port(port)
{
}

auto ServerBuilder::body_limit(size_t limit) -> ServerBuilder&
{
    _body_limit = limit;
    return *this;
}

auto ServerBuilder::buffer_limit(size_t limit) -> ServerBuilder&
{
    _buffer_limit = limit;
    return *this;
}

auto ServerBuilder::inactivity_timeout(size_t timeout) -> ServerBuilder&
{
    _inactivity_timeout = timeout;
    return *this;
}

auto ServerBuilder::error_page_handler(ErrorPageHandler error_page_handler) -> ServerBuilder&
{
    _error_page_handler = error_page_handler;
    return *this;
}

auto ServerBuilder::serve(RequestHandler request_handler) -> Server
{
    if (!_error_page_handler.has_value()) {
        _error_page_handler = [](Status const& status) {
            return BoxPtr<DefaultPageReader>::make(status);
        };
    }
    auto props = ServerProperties(move(request_handler), move(*_error_page_handler));

    props._body_limit = _body_limit.value_or(size_t(DEFAULT_BODY_LIMIT));
    props._buffer_limit = _buffer_limit.value_or(size_t(DEFAULT_BUFFER_LIMIT));
    props._inactivity_timeout = _inactivity_timeout.value_or(size_t(DEFAULT_INACTIVITY_TIMEOUT));

    auto server = Server(_ip, _port, props);
    return server;
}

// lmao @ clang-format indenting
// TODO: error handler for foreachfuture is not set
Server::Server(IpAddress address, uint16_t port, ServerProperties props)
    : _listener(TcpListener(address, port)
                    .for_each<TcpListener>([props](TcpStream&& stream) {
                        GlobalRuntime::spawn(
                            ConnectionFuture(
                                move(stream),
                                props));
                    }))
    , _properties(move(props))
{
}

auto Server::poll(Waker&& waker) -> PollResult<void>
{
    return _listener.poll(move(waker));
}

auto Server::bind(IpAddress addr, uint16_t port) -> ServerBuilder
{
    return ServerBuilder(addr, port);
}

Server::ConnectionFuture::ConnectionFuture(Server::ConnectionFuture&& other) noexcept
    : _request(move(other._request))
    , _response(move(other._response))
    , _copier(move(other._copier))
    , _stream(move(other._stream))
    , _parser(RequestParser(move(*other._parser), _stream.get_socket()))
    , _reader(move(other._reader))
    , _is_recovering(other._is_recovering)
    , _timeout(move(other._timeout))
    , _properties(move(other._properties))
{
}

Server::ConnectionFuture::ConnectionFuture(TcpStream&& stream, ServerProperties props)
    : _stream(move(stream))
    , _timeout(props._inactivity_timeout)
    , _properties(props)
{
    _parser = RequestParser(
        _stream.get_socket(),
        props._buffer_limit,
        props._body_limit);
}

void Server::ConnectionFuture::switch_to_respond_mode()
{
    // lol
    if (_request.has_value() && _request->get_method() == method::HEAD) {
        _response->drop_body();
    }
    _reader = ResponseReader(*_response);
    _copier = ioruntime::IoCopyFuture<void, void>(*_reader, _stream.get_socket());
    _timeout = TimeoutFuture(_properties._inactivity_timeout);
    _state = Respond;
}

// For handlers, HttpRequest is guaranteed to live until the end of the response
auto Server::ConnectionFuture::poll(Waker&& waker) -> PollResult<void>
{
    switch (_state) {
        case Listen: {
            try {
                auto poll_result = _parser->poll(Waker(waker));
                if (poll_result.is_ready()) {
                    // forward the request to the request handler
                    _request = move(poll_result.get());
                    _response = _properties._request_handler(*_request, _stream.get_addr());
                    // switch to respond mode
                    switch_to_respond_mode();
                    return poll(move(waker));
                }
                auto timer_result = _timeout->poll(Waker(waker));
                if (timer_result.is_ready()) {
                    throw TimeoutError();
                }
                // if we're woken up, it's active, so reset the timer
                _timeout = TimeoutFuture(_properties._inactivity_timeout);
                return PollResult<void>::pending();
            } catch (std::exception const& err) {
                WARNPRINT("Request handler failed: " << err.what());
                try {
                    auto builder = ResponseBuilder();
                    auto status = status::INTERNAL_SERVER_ERROR;

                    try {
                        throw;
                    } catch (RequestParser::UndeterminedLength const&) {
                        status = status::LENGTH_REQUIRED;
                    } catch (RequestParser::RequestUriExceededBuffer const&) {
                        status = status::URI_TOO_LONG;
                    } catch (RequestParser::BodyExceededLimit const&) {
                        status = status::PAYLOAD_TOO_LARGE;
                    } catch (RequestParser::GenericExceededBuffer const&) {
                        status = status::REQUEST_HEADER_FIELDS_TOO_LARGE;
                    } catch (RequestParser::InvalidMethod const&) {
                        status = status::NOT_IMPLEMENTED;
                    } catch (RequestParser::InvalidVersion const&) {
                        status = status::VERSION_NOT_SUPPORTED;
                    } catch (RequestParser::MalformedRequest const&) {
                        status = status::BAD_REQUEST;
                    } catch (RequestParser::UnexpectedEof const&) {
                        status = status::BAD_REQUEST;
                    } catch (RequestParser::BadTransferEncoding const&) {
                        status = status::NOT_IMPLEMENTED;
                    } catch (TimeoutError const&) {
                        status = status::REQUEST_TIMEOUT;
                        // this header is not required, but it's still nice to have
                        builder.header(header::CONNECTION, "close");
                    } catch (fs::FileNotFound const&) {
                        status = status::NOT_FOUND;
                    } catch (HandlerStatusError& hse) {
                        status = hse.status();
                        if (hse.headers().has_value()) {
                            builder.headers(move(*hse.headers()));
                        }
                    } catch (std::exception const& err) {
                        // status is already set to INTERNAL_SERVER_ERROR
                        WARNPRINT("Request handler failed: " << err.what());
                    }

                    builder
                        .status(status)
                        .body(_properties._error_page_handler(status));
                    _response = move(builder).build();
                    switch_to_respond_mode();
                    return poll(move(waker));
                } catch (...) {
                    // our error handler errored, close the connection
                    return PollResult<void>::ready();
                }
            }
        } break;
        case Respond: {
            try {
                auto copy_result = _copier->poll(Waker(waker));
                if (copy_result.is_ready()) {
                    return copy_result;
                } else {
                    if (_byte_activity != _copier->get_bytes_written()) {
                        // if we're woken up, it's active, so reset the timer
                        _timeout = TimeoutFuture(_properties._inactivity_timeout);
                        _byte_activity = _copier->get_bytes_written();
                    }
                }

                auto timer_result = _timeout->poll(Waker(waker));
                if (timer_result.is_ready()) {
                    WARNPRINT("timed out");
                    throw TimeoutError();
                }
                return copy_result;
            } catch (std::exception const& err) {
                if (_is_recovering) {
                    // we errored in recovery mode
                    // we can not recover from this, so stop responding
                    ERRORPRINT("Response error recovery errored: " << err.what());
                    return PollResult<void>::ready();
                } else {
                    WARNPRINT("Response copier failed: " << err.what());
                }
                if (!_copier->has_written_bytes()) {
                    // we haven't written any part of the response yet,
                    // so we can back out and send an error response
                    try {
                        auto status = status::INTERNAL_SERVER_ERROR;
                        try {
                            throw;
                        } catch (cgi::Cgi::CgiError const& cgi_err) {
                            // CGI "init" errors are thrown inside of the ResponseReader.
                            WARNPRINT("CGI error: " << cgi_err.what());
                            status = status::BAD_GATEWAY;
                        } catch (TimeoutError const&) {
                            status = status::GATEWAY_TIMEOUT;
                        } catch (...) {
                            // status is already set to INTERNAL_SERVER_ERROR
                        }
                        auto builder = ResponseBuilder();
                        builder
                            .status(status)
                            .body(_properties._error_page_handler(status));
                        _response = move(builder).build();
                        switch_to_respond_mode();
                        _is_recovering = true;
                        return poll(move(waker));
                    } catch (std::exception const& e) {
                        ERRORPRINT("Response recovery setup error: " << e.what());
                        // we can't switch into recovery mode,
                        // so stop responding
                        return PollResult<void>::ready();
                    }
                } else {
                    // we have already written part of the response,
                    // we can no longer change our response
                    return PollResult<void>::ready();
                }
            }
        } break;
        default: {
            throw std::logic_error("ConnectionFuture unreachable state");
        } break;
    }
}

Server::ConnectionFuture::~ConnectionFuture()
{
}

ServerProperties::ServerProperties(RequestHandler rh, ErrorPageHandler eh)
    : _request_handler(rh)
    , _error_page_handler(eh)
{
}

HandlerStatusError::HandlerStatusError(Status status)
    : std::runtime_error("Intentional status error")
    , _status(status)
    , _headers(option::nullopt)
{
}

HandlerStatusError::HandlerStatusError(Status status, Headers&& headers)
    : std::runtime_error("Intentional status error")
    , _status(status)
    , _headers(move(headers))
{
}

auto HandlerStatusError::status() const -> Status
{
    return _status;
}

auto HandlerStatusError::headers() -> optional<Headers>&
{
    return _headers;
}

} // namespace http