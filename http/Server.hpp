//
// Created by boat on 8/22/20.
//

#ifndef WEBSERV_HTTPSERVER_HPP
#define WEBSERV_HTTPSERVER_HPP

#include "../futures/ForEachFuture.hpp"
#include "../futures/IFuture.hpp"
#include "../ioruntime/TimeoutFuture.hpp"
#include "../net/TcpListener.hpp"
#include "../net/TcpStream.hpp"
#include "RequestParser.hpp"
#include "Response.hpp"

namespace {
using futures::ForEachFuture;
using futures::IFuture;
using ioruntime::TimeoutFuture;
using net::IpAddress;
using net::SocketAddr;
using net::TcpListener;
using net::TcpStream;
}

namespace http {

constexpr size_t DEFAULT_BODY_LIMIT = 8192;
constexpr size_t DEFAULT_BUFFER_LIMIT = 8192;
constexpr size_t DEFAULT_INACTIVITY_TIMEOUT = 10000;

using RequestHandler = std::function<Response(Request&, SocketAddr const&)>;
using ErrorPageHandler = std::function<BoxPtr<IAsyncRead>(Status&)>;

class TimeoutError final : public std::runtime_error {
public:
    TimeoutError();
};

class HandlerStatusError final : public std::runtime_error {
public:
    explicit HandlerStatusError(Status status);
    HandlerStatusError(Status status, Headers&& headers);
    [[nodiscard]] auto status() const -> Status;
    [[nodiscard]] auto headers() -> optional<Headers>&;
private:
    Status _status;
    optional<Headers> _headers;
};

class Server;
class ServerBuilder;
class ServerProperties {
    friend class Server;
    friend class ServerBuilder;
public:
    ServerProperties(RequestHandler rh, ErrorPageHandler eh);

private:
    RequestHandler _request_handler;
    ErrorPageHandler _error_page_handler;
    size_t _body_limit;
    size_t _buffer_limit;
    size_t _inactivity_timeout;
};

class ServerBuilder final {
    friend class Server;

public:
    ServerBuilder() = delete;
    auto serve(RequestHandler request_handler) -> Server;
    auto body_limit(size_t limit) -> ServerBuilder&;
    auto buffer_limit(size_t limit) -> ServerBuilder&;
    auto inactivity_timeout(size_t timeout) -> ServerBuilder&;
    auto error_page_handler(ErrorPageHandler error_page_handler) -> ServerBuilder&;

private:
    ServerBuilder(IpAddress ip, uint16_t port);
    IpAddress _ip;
    uint16_t _port;
    optional<size_t> _body_limit;
    optional<size_t> _buffer_limit;
    optional<size_t> _inactivity_timeout;
    optional<ErrorPageHandler> _error_page_handler;
};

class Server final : public IFuture<void> {
    friend class ServerBuilder;

private:
    class ConnectionFuture final : public IFuture<void> {
    public:
        ConnectionFuture() = delete;
        ~ConnectionFuture() override;
        ConnectionFuture(ConnectionFuture&& other) noexcept;
        explicit ConnectionFuture(TcpStream&& stream, ServerProperties props);
        auto poll(Waker&& waker) -> PollResult<void> override;

    private:
        // methods
        void switch_to_respond_mode();

        // members
        enum State {
            Listen,
            Respond
        } _state
            = Listen;
        optional<Request> _request;
        optional<Response> _response;
        optional<ioruntime::IoCopyFuture> _copier;
        net::TcpStream _stream;
        optional<RequestParser> _parser;
        optional<ResponseReader> _reader;
        bool _is_recovering = false;
        TimeoutFuture _timeout;
        ServerProperties _properties;
    };

public:
    Server() = delete;
    auto poll(Waker&& waker) -> PollResult<void> override;
    static auto bind(IpAddress addr, uint16_t port) -> ServerBuilder;

private:
    explicit Server(IpAddress address, uint16_t port, ServerProperties props);
    ForEachFuture<TcpListener> _listener;
    ServerProperties _properties;
};

}

//#include "Server.ipp"

#endif //WEBSERV_HTTPSERVER_HPP
