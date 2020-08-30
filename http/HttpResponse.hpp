//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "DefaultPageBody.hpp"
#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpRfcConstants.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
#include <map>
#include <string>

namespace http {

class HttpResponse;

class HttpResponseBuilder {
public:
    HttpResponseBuilder();
    HttpResponseBuilder& version(HttpVersion version);
    HttpResponseBuilder& status(HttpStatus status);
    HttpResponseBuilder& header(HttpHeaderName name, HttpHeaderValue value);
    HttpResponseBuilder& body(BoxPtr<ioruntime::IAsyncRead>&& body);
    HttpResponse build();

private:
    std::map<HttpHeaderName, HttpHeaderValue> _headers;
    HttpVersion _version = HTTP_VERSION_1_1;
    HttpStatus _status;
    BoxPtr<ioruntime::IAsyncRead> _body;
};

class HttpResponse {
public:

    HttpResponse();
    explicit HttpResponse(std::map<HttpHeaderName, HttpHeaderValue>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body);
    HttpResponse(HttpResponse&& other) noexcept;

    HttpResponse& operator=(HttpResponse&& other) noexcept;
    virtual ~HttpResponse();
    PollResult<void> poll_respond(ioruntime::Socket& socket, Waker&& waker);
    bool write_response(ioruntime::Socket& socket, Waker&& waker);
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
        WriteBody
    };
    char buf[128] { };
    State state { WriteStatusVersion };
    std::string_view current;
    ssize_t written { 0 };
    std::map<HttpHeaderName, HttpHeaderValue> response_headers;
    std::map<HttpHeaderName, HttpHeaderValue>::const_iterator header_it;
    HttpStatus response_status;
    HttpVersion response_version;
    BoxPtr<ioruntime::IAsyncRead> response_body;
};

}


#endif //WEBSERV_HTTPRESPONSE_HPP
