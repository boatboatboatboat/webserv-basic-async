//
// Created by boat on 8/22/20.
//

#ifndef WEBSERV_HTTPSERVER_HPP
#define WEBSERV_HTTPSERVER_HPP

#include "../futures/ForEachFuture.hpp"
#include "../futures/IFuture.hpp"
#include "../ioruntime/TcpListener.hpp"
#include "../ioruntime/TcpStream.hpp"
#include "HttpResponse.hpp"
#include "ParserFuture.hpp"

using futures::ForEachFuture;
using futures::IFuture;
using ioruntime::TcpListener;
using ioruntime::TcpStream;

namespace http {
template <typename RH>
class HttpServer : public IFuture<void> {
private:
    class HttpConnectionFuture : public IFuture<void> {
    public:
        HttpConnectionFuture() = delete;
        ~HttpConnectionFuture() override;
        HttpConnectionFuture(HttpConnectionFuture&& other) noexcept;
        explicit HttpConnectionFuture(TcpStream&& stream);
        PollResult<void> poll(Waker&& waker) override;

    private:
        enum State {
            Listen,
            Respond
        };
        State state = Listen;
        HttpRequest req;
        HttpResponse res;
        TcpStream stream;
        ParserFuture parser;
        RH handler;
    };

public:
    HttpServer() = delete;
    explicit HttpServer(uint16_t port, RH fn);
    PollResult<void> poll(Waker&& waker) override;
    static void handle_connection(TcpStream& stream);
    static void handle_exception(std::exception& e);

private:
    ForEachFuture<TcpListener, TcpStream> listener;
    static RH handler;
};

}

#include "HttpServer.ipp"

#endif //WEBSERV_HTTPSERVER_HPP
