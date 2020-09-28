//
// Created by boat on 8/22/20.
//

#include "../ioruntime/GlobalRuntime.hpp"
#include "../net/TcpListener.hpp"
#include "../net/TcpStream.hpp"
#include "HttpHeader.hpp"
#include "HttpResponse.hpp"
#include "HttpServer.hpp"

using ioruntime::GlobalRuntime;

namespace http {

template <typename RH, typename EH>
HttpServer<RH, EH>::HttpServer(net::IpAddress address, uint16_t port, RH rh, EH eh)
    : listener(net::TcpListener(address, port).for_each<net::TcpListener>(handle_connection, handle_exception))
{
    // If we don't pass RH/EH as a parameter,
    // we would have to write HttpServer<decltype(SOME_LAMBDA)>(PORT)
    // instead of HttpServer(PORT, SOME_LAMBDA)
    // We only need the type of RH/EH to set the request_handler function,
    // not the value.
    (void)rh;
    (void)eh;
}

template <typename RH, typename EH>
PollResult<void> HttpServer<RH, EH>::poll(Waker&& waker)
{
    return listener.poll(std::move(waker));
}

template <typename RH, typename EH>
void HttpServer<RH, EH>::handle_connection(net::TcpStream& stream)
{
    GlobalRuntime::spawn(HttpConnectionFuture(std::move(stream)));
}

template <typename RH, typename EH>
void HttpServer<RH, EH>::handle_exception(std::exception& e)
{
    (void)e;
    TRACEPRINT("HttpServer: " << e.what());
}

template <typename RH, typename EH>
PollResult<void> HttpServer<RH, EH>::HttpConnectionFuture::poll(Waker&& waker)
{
    switch (state) {
    case Listen: {
        try {
            TRACEPRINT("connection listening");
            auto poll_result = parser->poll(Waker(waker));
            if (poll_result.is_ready()) {
                req = poll_result.get();
                state = Respond;
                res = std::move(request_handler(*req));
                return poll(std::move(waker));
            }
            TRACEPRINT("connection listening match timer");
            auto timer_result = timeout.poll(Waker(waker));
            if (timer_result.is_ready()) {
                throw ConnectionTimeout();
            }
        } catch (std::exception& e) {
            state = Respond;
            auto builder = HttpResponseBuilder();
            builder.header(http::header::CONTENT_TYPE, "text/html; charset=utf8");

            auto status = HTTP_STATUS_INTERNAL_SERVER_ERROR;

            try {
                throw;
            } catch (StreamingHttpRequestParser::UndeterminedLength&) {
                status = HTTP_STATUS_LENGTH_REQUIRED;
            } catch (StreamingHttpRequestParser::RequestUriExceededBuffer&) {
                status = HTTP_STATUS_URI_TOO_LONG;
            } catch (StreamingHttpRequestParser::BodyExceededLimit&) {
                status = HTTP_STATUS_PAYLOAD_TOO_LARGE;
            } catch (StreamingHttpRequestParser::ParserError&) {
                status = HTTP_STATUS_BAD_REQUEST;
            } catch (ConnectionTimeout&) {
                status = HTTP_STATUS_REQUEST_TIMEOUT;
            } catch (fs::File::FileNotFound&) {
                status = HTTP_STATUS_NOT_FOUND;
            } catch (std::exception& e) {
                // we should still catch the errors
            }

            builder.status(status);

            try {
                builder.body(std::move(error_handler(status)));
            } catch (...) {
                // well, it failed, try to load the 500 internal server error one
                status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                try {
                    builder.body(std::move(error_handler(status)));
                } catch (...) {
                    // well that also failed, let it use the default page instead
                }
            }
            res = std::move(builder).build();
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

template <typename RH, typename EH>
HttpServer<RH, EH>::HttpConnectionFuture::HttpConnectionFuture(net::TcpStream&& pstream)
    : req()
    , res()
    , stream(std::move(pstream))
    , parser(optional<StreamingHttpRequestParser>(StreamingHttpRequestParser(stream.get_socket(), 8192, 8192)))
    , timeout(5000)
{
}

template <typename RH, typename EH>
HttpServer<RH, EH>::HttpConnectionFuture::~HttpConnectionFuture()
{
}

template <typename RH, typename EH>
HttpServer<RH, EH>::HttpConnectionFuture::HttpConnectionFuture(HttpServer::HttpConnectionFuture&& other) noexcept
    : state(other.state)
    , req(std::move(other.req))
    , res(std::move(other.res))
    , stream(std::move(other.stream))
    , parser(StreamingHttpRequestParser(std::move(*other.parser), stream.get_socket()))
    , timeout(std::move(other.timeout))
    , request_handler(other.request_handler)
    , error_handler(other.error_handler)
{
}
}
