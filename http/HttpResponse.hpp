//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "../net/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
#include <map>
#include <string>

// Get the max size of the buffer to store a chunked transfer size in
static inline constexpr auto get_max_num_size(size_t s) -> size_t
{
    // gets the length of a number in hex notation, + 1
    size_t n = 0;
    do {
        n += 1;
        s /= 16;
    } while (s);
    return n + 1;
}

namespace http {

class HttpResponse;

class HttpResponseBuilder {
public:
    HttpResponseBuilder();
    auto version(HttpVersion version) -> HttpResponseBuilder&;
    auto status(HttpStatus status) -> HttpResponseBuilder&;
    auto header(HttpHeaderName name, const HttpHeaderValue& value) -> HttpResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body) -> HttpResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> HttpResponseBuilder&;
    auto build() -> HttpResponse;

private:
    std::map<HttpHeaderName, HttpHeaderValue> _headers;
    HttpVersion _version = HTTP_VERSION_1_1;
    HttpStatus _status;
    BoxPtr<ioruntime::IAsyncRead> _body;
};

class HttpResponse {
public:
    HttpResponse();
    explicit HttpResponse(
        std::map<HttpHeaderName, HttpHeaderValue>&& response_headers,
        HttpStatus status,
        BoxPtr<ioruntime::IAsyncRead>&& response_body);
    HttpResponse(HttpResponse&& other) noexcept;

    auto operator=(HttpResponse&& other) noexcept -> HttpResponse&;
    virtual ~HttpResponse();
    auto poll_respond(net::Socket& socket, Waker&& waker) -> PollResult<void>;
    auto write_response(net::Socket& socket, Waker&& waker) -> bool;

private:
    enum State {
        WriteStatusVersion,
        WriteStatusSpace1,
        WriteStatusCode,
        WriteStatusSpace2,
        WriteStatusMessage,
        WriteStatusCRLF,
        WriteHeaderName,
        WriteHeaderSplit,
        WriteHeaderValue,
        WriteHeaderCRLF,
        WriteSeperatorCLRF,
        ReadBody,
        WriteBody,
        WriteChunkedBodySize,
        WriteChunkedBodyCLRF1,
        WriteChunkedBody,
        WriteChunkedBodyCLRF2,
        WriteChunkedBodyEof
    };
    char buf[8192] {};
    char num[get_max_num_size(sizeof(buf))] {};
    State state { WriteStatusVersion };
    std::string_view current;
    std::string_view body_view;
    ssize_t written { 0 };
    std::map<HttpHeaderName, HttpHeaderValue> response_headers;
    std::map<HttpHeaderName, HttpHeaderValue>::const_iterator header_it;
    HttpStatus response_status;
    HttpVersion response_version;
    BoxPtr<ioruntime::IAsyncRead> response_body;
};

}

#endif //WEBSERV_HTTPRESPONSE_HPP
