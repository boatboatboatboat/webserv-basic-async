//
// Created by boat on 8/22/20.
//

#include "../cgi/Cgi.hpp"
#include "../ioruntime/GlobalRuntime.hpp"
#include "../net/TcpListener.hpp"
#include "../net/TcpStream.hpp"
#include "HttpHeader.hpp"
#include "HttpResponse.hpp"
#include "HttpServer.hpp"

using ioruntime::GlobalRuntime;

namespace http {

template <typename RH>
HttpServer<RH>::HttpServer(net::IpAddress address, uint16_t port, RH fn)
    : listener(net::TcpListener(address, port).for_each<net::TcpListener>(handle_connection, handle_exception))
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
void HttpServer<RH>::handle_connection(net::TcpStream& stream)
{
    GlobalRuntime::spawn(HttpConnectionFuture(std::move(stream)));
}

template <typename RH>
void HttpServer<RH>::handle_exception(std::exception& e)
{
    (void)e;
    TRACEPRINT("HttpServer: " << e.what());
}

template <typename RH>
PollResult<void> HttpServer<RH>::HttpConnectionFuture::poll(Waker&& waker)
{
    switch (state) {
    case Listen: {
        try {
            TRACEPRINT("connection listening");
            auto poll_result = parser->poll(Waker(waker));
            if (poll_result.is_ready()) {
                req = poll_result.get();
                state = Respond;
                try {
                    res = std::move(handler(*req));
                } catch (cgi::Cgi::CgiError&) {
                    state = Respond;
                    res = HttpResponseBuilder()
                              .status(HTTP_STATUS_BAD_GATEWAY)
                              .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                              .build();
                    return poll(std::move(waker));
                } catch (std::exception& e) {
                    WARNPRINT("request handler error: " << e.what());
                    res = HttpResponseBuilder()
                              .status(HTTP_STATUS_INTERNAL_SERVER_ERROR)
                              .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                              .build();
                }
                return poll(std::move(waker));
            }
            TRACEPRINT("connection listening match timer");
            auto timer_result = timeout.poll(Waker(waker));
            if (timer_result.is_ready()) {
                // timeout
                state = Respond;
                // FIXME: gateway timeout requires connection header
                res = HttpResponseBuilder()
                          .status(HTTP_STATUS_GATEWAY_TIMEOUT)
                          .build();
                WARNPRINT("Request timed out");
                return poll(std::move(waker));
            }
        } catch (StreamingHttpRequestParser::UndeterminedLength& e) {
            state = Respond;
            res = HttpResponseBuilder()
                      .status(HTTP_STATUS_LENGTH_REQUIRED)
                      .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                      .build();
            return poll(std::move(waker));
        } catch (StreamingHttpRequestParser::RequestUriExceededBuffer& e) {
            state = Respond;
            res = HttpResponseBuilder()
                      .status(HTTP_STATUS_URI_TOO_LONG)
                      .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                      .build();
            return poll(std::move(waker));
        } catch (StreamingHttpRequestParser::BodyExceededLimit& e) {
            state = Respond;
            res = HttpResponseBuilder()
                      .status(HTTP_STATUS_PAYLOAD_TOO_LARGE)
                      .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                      .build();
            return poll(std::move(waker));
        } catch (StreamingHttpRequestParser::ParserError& e) {
            state = Respond;
            res = HttpResponseBuilder()
                      .status(HTTP_STATUS_BAD_REQUEST)
                      .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                      .build();
            return poll(std::move(waker));
        } catch (std::exception& e) {
            state = Respond;
            res = HttpResponseBuilder()
                      .status(HTTP_STATUS_INTERNAL_SERVER_ERROR)
                      .header(http::header::CONTENT_TYPE, "text/html; charset=utf8")
                      .build();
            return poll(std::move(waker));
        }
        return PollResult<void>::pending();
    } break;
    case Respond: {
        try {
            return res.poll_respond(stream.get_socket(), std::move(waker));
        } catch (std::exception& e) {
            WARNPRINT("response error: " << e.what());
            return PollResult<void>::ready();
        }
    } break;
    default: {
        throw std::runtime_error("HttpServer::HttpConnectionFuture: poll: unreachable state");
    } break;
    }
}

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::HttpConnectionFuture(net::TcpStream&& pstream)
    : req()
    , res()
    , stream(std::move(pstream))
    , parser(optional<StreamingHttpRequestParser>(StreamingHttpRequestParser(stream.get_socket(), 8192, 8192)))
    , timeout(5000)
{
}

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::~HttpConnectionFuture()
{
}

template <typename RH>
HttpServer<RH>::HttpConnectionFuture::HttpConnectionFuture(HttpServer::HttpConnectionFuture&& other) noexcept
    : state(other.state)
    , req(std::move(other.req))
    , res(std::move(other.res))
    , stream(std::move(other.stream))
    , parser(StreamingHttpRequestParser(std::move(*other.parser), stream.get_socket()))
    , timeout(std::move(other.timeout))
    , handler(other.handler)
{
}
}
