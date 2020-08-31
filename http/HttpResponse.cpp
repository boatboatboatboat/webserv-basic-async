#include "HttpResponse.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "StringBody.hpp"

namespace http {

HttpResponse::~HttpResponse() = default;

HttpResponse::HttpResponse()
    : response_headers()
    , response_status(HTTP_STATUS_IM_A_TEAPOT)
    , response_version(HTTP_VERSION_1_1)
    , response_body(nullptr)
{
}

HttpResponse::HttpResponse(std::map<HttpHeaderName, HttpHeaderValue>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body)
    : response_headers(std::move(response_headers))
    , response_status(status)
    , response_version(HTTP_VERSION_1_1)
    , response_body(std::move(response_body))
{
    if (this->response_body.get() == nullptr) {
        // fixme: _body is BoxPtr which is allocated, memory alloc error can't recover properly
        this->response_body = BoxPtr<DefaultPageBody>::make(status);
    }
}

bool HttpResponse::write_response(net::Socket& socket, Waker&& waker)
{
    auto str = current.data() + written;
    auto len = current.length() - written;

    auto poll_result = socket.poll_write(str, len, std::move(waker));

    if (poll_result.is_ready()) {
        auto result = poll_result.get();

        if (result < 0) {
            throw std::runtime_error("HttpResponse: write_respond: status write error");
        }
        written += result;

        if (written == (ssize_t)current.length()) {
            written = 0;
            return true;
        }
    }
    return false;
}

PollResult<void> HttpResponse::poll_respond(net::Socket& socket, Waker&& waker)
{
    TRACEPRINT("responder state: " << state);
    switch (state) {
    case WriteStatusVersion: {
        current = this->response_version.version_string;
    } break;
    case WriteStatusSpace1:
    case WriteStatusSpace2: {
        current = HTTP_SP;
    } break;
    case WriteStatusCode: {
        std::sprintf(buf, "%u", this->response_status.code);
        current = buf;
    } break;
    case WriteStatusCRLF: {
        current = HTTP_CRLF;
        header_it = response_headers.begin();
    } break;
    case WriteStatusMessage: {
        current = this->response_status.message;
    } break;
    case WriteHeaderName: {
        if (header_it == response_headers.end()) {
            state = WriteSeperatorCLRF;
            return poll_respond(socket, std::move(waker));
        }
        current = header_it->first;
    } break;
    case WriteHeaderSplit: {
        current = ": ";
    } break;
    case WriteHeaderValue: {
        current = header_it->second;
    } break;
    case WriteSeperatorCLRF:
    case WriteHeaderCRLF: {
        current = HTTP_CRLF;
    } break;
    case ReadBody: {
        auto poll_result = response_body->poll_read(buf, sizeof(buf), Waker(waker));
        if (poll_result.is_pending())
            return PollResult<void>::pending();
        if (poll_result.get() == 0) {
            INFOPRINT("end of body");
            return PollResult<void>::ready();
        }
        current = std::string_view(buf, poll_result.get());
        state = WriteBody;
        return poll_respond(socket, std::move(waker));
    } break;
    case WriteBody: {
    //    current = buf;
    } break;
    }

    if (write_response(socket, Waker(waker))) {
        if (state != WriteHeaderCRLF && state != WriteBody) {
            state = static_cast<State>(static_cast<int>(state) + 1);
        } else if (state == WriteBody) {
            state = ReadBody;
        } else if (state == WriteHeaderCRLF) {
            ++header_it;
            state = WriteHeaderName;
        }
        return poll_respond(socket, std::move(waker));
    }
    return PollResult<void>::pending();
}

HttpResponse::HttpResponse(HttpResponse&& other) noexcept
    : state(other.state)
    , current(other.current)
    , response_headers(std::move(other.response_headers))
    , header_it(other.header_it)
    , response_status(other.response_status)
    , response_version(other.response_version)
    , response_body(std::move(other.response_body))
{
}

HttpResponse& HttpResponse::operator=(HttpResponse&& other) noexcept
{
    if (&other == this) {
        return *this;
    }

    this->state = other.state;
    this->current = std::move(other.current);
    this->response_headers = std::move(other.response_headers);
    this->header_it = other.header_it;
    this->response_status = other.response_status;
    this->response_body = std::move(other.response_body);
    this->response_version = other.response_version;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::status(HttpStatus status)
{
    _status = status;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::header(HttpHeaderName name, HttpHeaderValue value)
{
    _headers.insert(std::pair<HttpHeaderName, HttpHeaderValue>(name, value));
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body)
{
    _body = std::move(body);
    return *this;
}

HttpResponse HttpResponseBuilder::build()
{
    return HttpResponse(std::move(_headers), _status, std::move(_body));
}

HttpResponseBuilder::HttpResponseBuilder()
    : _status(HTTP_STATUS_IM_A_TEAPOT)
    , _body(nullptr)
{
}

HttpResponseBuilder& HttpResponseBuilder::version(HttpVersion version)
{
    _version = version;
    return *this;
}

}