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

using futures::ForEachFuture;
using futures::IFuture;
using ioruntime::TimeoutFuture;
using net::TcpStream;
using ioruntime::TimeoutFuture;

namespace http {
template <typename RH>
class HttpServer : public IFuture<void> {
private:
    class HttpConnectionFuture : public IFuture<void> {
    public:
        HttpConnectionFuture() = delete;
        ~HttpConnectionFuture() override;
        HttpConnectionFuture(HttpConnectionFuture&& other) noexcept;
        explicit HttpConnectionFuture(net::TcpStream&& stream);
        PollResult<void> poll(Waker&& waker) override;

    private:
        enum State {
            Listen,
            Respond
        };
        State state = Listen;
        HttpRequest req;
        HttpResponse res;
        net::TcpStream stream;
        ParserFuture parser;
        TimeoutFuture timeout;
        RH handler;
    };

public:
    HttpServer() = delete;
    explicit HttpServer(net::IpAddress address, uint16_t port, RH fn);
    PollResult<void> poll(Waker&& waker) override;
    static void handle_connection(net::TcpStream& stream);
    static void handle_exception(std::exception& e);

private:
    ForEachFuture<net::TcpListener, net::TcpStream> listener;
    static RH handler;
};

}

#include "HttpServer.ipp"

#endif //WEBSERV_HTTPSERVER_HPP
