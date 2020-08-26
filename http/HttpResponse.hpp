//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include <map>
#include <string>

namespace http {
struct HttpStatus {
    unsigned int code;
    const char* message;
};

class HttpResponse;

class HttpResponseBuilder {
public:
    HttpResponseBuilder();
    HttpResponseBuilder& status(HttpStatus status);
    HttpResponseBuilder& header(std::string const& name, std::string const& value);
    HttpResponseBuilder& body(BoxPtr<ioruntime::IAsyncRead>&& body);
    HttpResponse build();

private:
    std::map<std::string, std::string> response_headers;
    HttpStatus response_status;
    BoxPtr<ioruntime::IAsyncRead> response_body;
};

class HttpResponse {
public:
    static const HttpStatus HTTP_STATUS_CONTINUE;
    static const HttpStatus HTTP_STATUS_SWITCHING_PROTOCOL;
    static const HttpStatus HTTP_STATUS_OK;
    static const HttpStatus HTTP_STATUS_MOVED_PERMANENTLY;
    static const HttpStatus HTTP_STATUS_BAD_REQUEST;
    static const HttpStatus HTTP_STATUS_UNAUTHORIZED;
    static const HttpStatus HTTP_STATUS_FORBIDDEN;
    static const HttpStatus HTTP_STATUS_NOT_FOUND;
    static const HttpStatus HTTP_STATUS_METHOD_NOT_ALLOWED;
    static const HttpStatus HTTP_STATUS_REQUEST_TIMEOUT;
    static const HttpStatus HTTP_STATUS_REQUEST_URI_TOO_LONG;
    static const HttpStatus HTTP_STATUS_INTERNAL_SERVER_ERROR;
    static const HttpStatus HTTP_STATUS_NOT_IMPLEMENTED;
    static const HttpStatus HTTP_STATUS_BAD_GATEWAY;
    static const HttpStatus HTTP_STATUS_GATEWAY_TIMEOUT;
    static const HttpStatus HTTP_STATUS_VERSION_NOT_SUPPORTED;
    static const HttpStatus HTTP_STATUS_SERVICE_UNAVAILABLE;
    static const HttpStatus HTTP_STATUS_REQUEST_IM_A_TEAPOT;

    HttpResponse();
    explicit HttpResponse(std::map<std::string, std::string>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body);
    HttpResponse(HttpResponse&& other) noexcept;

    HttpResponse& operator=(HttpResponse&& other) noexcept;
    virtual ~HttpResponse();
    PollResult<void> poll_respond(ioruntime::Socket& socket, Waker&& waker);
private:
    enum State {
        WriteStatus,
        WriteHeaders,
        WriteBody
    };
    State state = WriteStatus;
    std::string current;
    ssize_t written {};
    std::map<std::string, std::string> response_headers;
    std::map<std::string, std::string>::const_iterator header_it;
    HttpStatus response_status;
    BoxPtr<ioruntime::IAsyncRead> response_body;
};

}

#endif //WEBSERV_HTTPRESPONSE_HPP
