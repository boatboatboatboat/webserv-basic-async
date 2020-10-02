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
auto HttpServer<RH>::poll(Waker&& waker) -> PollResult<void>
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
auto HttpServer<RH>::HttpConnectionFuture::poll(Waker&& waker) -> PollResult<void>
{
    switch (state) {
    case Listen: {
        try {
            TRACEPRINT("connection listening");
            auto poll_result = parser->poll(Waker(waker));
            if (poll_result.is_ready()) {
                req = poll_result.get();
                state = Respond;
                res = std::move(handler(*req, stream.get_addr()));
                return poll(std::move(waker));
            }
            TRACEPRINT("connection listening match timer");
            auto timer_result = timeout.poll(Waker(waker));
            if (timer_result.is_ready()) {
                throw TimeoutError();
            }
        } catch (std::exception const&) {
            state = Respond;
            auto builder = HttpResponseBuilder();

            builder.header(header::CONTENT_TYPE, "text/html; charset=utf8");
            auto status = status::INTERNAL_SERVER_ERROR;

            try {
                throw;
            } catch (HttpRequestParser::UndeterminedLength const&) {
                status = status::LENGTH_REQUIRED;
            } catch (HttpRequestParser::RequestUriExceededBuffer const&) {
                status = status::URI_TOO_LONG;
            } catch (HttpRequestParser::BodyExceededLimit const&) {
                status = status::PAYLOAD_TOO_LARGE;
            } catch (HttpRequestParser::GenericExceededBuffer const&) {
                status = status::REQUEST_HEADER_FIELDS_TOO_LARGE;
            } catch (HttpRequestParser::InvalidMethod const&) {
                status = status::NOT_IMPLEMENTED;
            } catch (HttpRequestParser::InvalidVersion const&) {
                status = status::VERSION_NOT_SUPPORTED;
            } catch (HttpRequestParser::MalformedRequest const&) {
                status = status::BAD_REQUEST;
            } catch (HttpRequestParser::UnexpectedEof const&) {
                status = status::BAD_REQUEST;
            } catch (HttpRequestParser::BadTransferEncoding const&) {
                status = status::NOT_IMPLEMENTED;
            } catch (TimeoutError const&) {
                status = status::REQUEST_TIMEOUT;
                // this header is not required
                builder.header(header::CONNECTION, "close");
            } catch (...) {
                // status is already set to INTERNAL_SERVER_ERROR
            }
            res = builder.status(status).build();
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
    , parser(optional<HttpRequestParser>(HttpRequestParser(stream.get_socket(), 8192, 8192)))
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
    , parser(HttpRequestParser(std::move(*other.parser), stream.get_socket()))
    , timeout(std::move(other.timeout))
    , handler(other.handler)
{
}
}
