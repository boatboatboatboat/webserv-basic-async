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
#include "HttpResponse.hpp"
#include "ParserFuture.hpp"
#include "StreamingHttpRequestParser.hpp"

using futures::ForEachFuture;
using futures::IFuture;
using ioruntime::TimeoutFuture;
using net::TcpStream;

namespace http {

class ConnectionTimeout : public std::runtime_error {
public:
    ConnectionTimeout() : std::runtime_error("Connection timed out") {}
private:
};

template <typename RH, typename EH>
class HttpServer : public IFuture<void> {
private:
    class HttpConnectionFuture : public IFuture<void> {
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
        };
        State state = Listen;
        optional<StreamingHttpRequest> req;
        HttpResponse res;
        net::TcpStream stream;
        optional<StreamingHttpRequestParser> parser;
        TimeoutFuture timeout;
        RH request_handler;
        EH error_handler;
    };

public:
    HttpServer() = delete;
    explicit HttpServer(net::IpAddress address, uint16_t port, RH rh, EH eh);
    PollResult<void> poll(Waker&& waker) override;
    static void handle_connection(net::TcpStream& stream);
    static void handle_exception(std::exception& e);

private:
    ForEachFuture<net::TcpListener, net::TcpStream> listener;
    static RH request_handler;
    static EH error_handler;
};

}

#include "HttpServer.ipp"

#endif //WEBSERV_HTTPSERVER_HPP
