//
// Created by boat on 8/22/20.
//

#include "../ioruntime/GlobalRuntime.hpp"
#include "HttpServer.hpp"

using ioruntime::GlobalRuntime;

namespace http {

template <typename RH>
HttpServer<RH>::HttpServer(uint16_t port, RH fn)
    : listener(TcpListener(port).for_each<TcpListener>(handle_connection))
{
    // If we don't pass RH as a parameter,
    // we would have to write HttpServer<decltype(SOME_LAMBDA)>(PORT)
    // instead of HttpServer(PORT, SOME_LAMBDA)
    // We only need the type of RH to set the handler function,
    // not the value
    (void)fn;
}

template <typename RH>
PollResult<void> HttpServer<RH>::poll(Waker&& waker)
{
    return listener.poll(std::move(waker));
}

template <typename RH>
void HttpServer<RH>::handle_connection(TcpStream& stream)
{
    GlobalRuntime::spawn(HttpConnectionFuture(std::move(stream)));
}

template <typename RH>
PollResult<void> HttpServer<RH>::HttpConnectionFuture::poll(Waker&& waker)
{
    switch (state) {
    case Listen: {
        auto poll_result = parser.poll(Waker(waker));
        if (poll_result.is_ready()) {
            req = poll_result.get();
            state = Respond;
            res = std::move(handler(req));
            return poll(std::move(waker));
        }
        return PollResult<void>::pending();
    } break;
    case Respond: {
        return res.poll_respond(stream.get_socket(), std::move(waker));
    } break;
    default: {
        throw std::runtime_error("HttpServer::HttpConnectionFuture: poll: unreachable state");
    } break;
    }
}

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::HttpConnectionFuture(TcpStream&& pstream)
    : req()
    , res()
    , stream(std::move(pstream))
    , parser(&stream)
{
}

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::~HttpConnectionFuture() = default;

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::HttpConnectionFuture(HttpServer::HttpConnectionFuture&& other) noexcept
    : state(other.state)
    , req(std::move(other.req))
    , res(std::move(other.res))
    , stream(std::move(other.stream))
    , parser(std::move(other.parser))
    , handler(other.handler)
{
    parser.stream = &stream;
}

}
