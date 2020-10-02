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

using futures::ForEachFuture;
using futures::IFuture;
using ioruntime::TimeoutFuture;
using net::TcpStream;

namespace http {

class TimeoutError final : public std::runtime_error {
public:
    TimeoutError();
};

template <typename RH>
class HttpServer final : public IFuture<void> {
private:
    class HttpConnectionFuture final : public IFuture<void> {
    public:
        HttpConnectionFuture() = delete;
        ~HttpConnectionFuture() override;
        HttpConnectionFuture(HttpConnectionFuture&& other) noexcept;
        explicit HttpConnectionFuture(net::TcpStream&& stream);
        auto poll(Waker&& waker) -> PollResult<void> override;

    private:
        enum State {
            Listen,
            Respond
        } state
            = Listen;
        optional<Request> req;
        LegacyHttpResponse res;
        net::TcpStream stream;
        optional<RequestParser> parser;
        TimeoutFuture timeout;
        RH handler;
    };

public:
    HttpServer() = delete;
    explicit HttpServer(net::IpAddress address, uint16_t port, RH fn);
    auto poll(Waker&& waker) -> PollResult<void> override;
    static void handle_connection(net::TcpStream& stream);
    static void handle_exception(std::exception& e);

private:
    ForEachFuture<net::TcpListener, net::TcpStream> listener;
    static RH _handler;
};

}

#include "HttpServer.ipp"

#endif //WEBSERV_HTTPSERVER_HPP
