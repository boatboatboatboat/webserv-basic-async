#include "HttpResponse.hpp"
#include "../ioruntime/Socket.hpp"
#include "DefaultPageBody.hpp"
#include "HttpRequest.hpp"
#include "StringBody.hpp"

// FIXME: illegal headers
#include <fstream>
#include <sstream>
#include <utility>

namespace http {

const HttpStatus HttpResponse::HTTP_STATUS_CONTINUE = { .code = 100, .message = "Continue" };
const HttpStatus HttpResponse::HTTP_STATUS_SWITCHING_PROTOCOL = { .code = 101, .message = "Switching Protocols" };
const HttpStatus HttpResponse::HTTP_STATUS_OK = { .code = 200, .message = "OK" };
const HttpStatus HttpResponse::HTTP_STATUS_MOVED_PERMANENTLY = { .code = 301, .message = "Moved Permanently" };
const HttpStatus HttpResponse::HTTP_STATUS_BAD_REQUEST = { .code = 400, .message = "Bad Request" };
const HttpStatus HttpResponse::HTTP_STATUS_UNAUTHORIZED = { .code = 401, .message = "Unauthorized" };
const HttpStatus HttpResponse::HTTP_STATUS_FORBIDDEN = { .code = 403, .message = "Forbidden" };
const HttpStatus HttpResponse::HTTP_STATUS_NOT_FOUND = { .code = 404, .message = "Not Found" };
const HttpStatus HttpResponse::HTTP_STATUS_METHOD_NOT_ALLOWED = { .code = 405 , .message = "Method Not Allowed" };
const HttpStatus HttpResponse::HTTP_STATUS_REQUEST_TIMEOUT = { .code = 408 , .message = "Request Timeout" };
const HttpStatus HttpResponse::HTTP_STATUS_REQUEST_URI_TOO_LONG = { .code = 414, .message = "URI Too Long" };
const HttpStatus HttpResponse::HTTP_STATUS_REQUEST_IM_A_TEAPOT = { .code = 418, .message = "I'm a Teapot" };
const HttpStatus HttpResponse::HTTP_STATUS_INTERNAL_SERVER_ERROR = { .code = 500, .message = "Internal Server Error" };
const HttpStatus HttpResponse::HTTP_STATUS_NOT_IMPLEMENTED = { .code = 501, .message = "Not Implemented" };
const HttpStatus HttpResponse::HTTP_STATUS_BAD_GATEWAY = { .code = 502, .message = "Bad Gateway" };
const HttpStatus HttpResponse::HTTP_STATUS_SERVICE_UNAVAILABLE = { .code = 503, .message = "Service Unavailable" };
const HttpStatus HttpResponse::HTTP_STATUS_GATEWAY_TIMEOUT = { .code = 504, .message = "Gateway Timeout" };
const HttpStatus HttpResponse::HTTP_STATUS_VERSION_NOT_SUPPORTED = { .code = 505, .message = "HTTP Version Not Supported" };

// FIXME: Global with a ctor that can throw
static std::string lineTerminator = "\r\n";

//HttpResponse::HttpResponse(HttpRequest* request)
//{
//    this->request = request;
//    this->status = 200;
//    this->header_set = false;
//}

HttpResponse::~HttpResponse() = default;

//void HttpResponse::addHeader(const std::string& name, const std::string& value)
//{
//    if (!this->header_set)
//        this->response_headers.insert(std::pair<std::string, std::string>(name, value));
//}

//void HttpResponse::close()
//{
//    if (!this->header_set)
//        this->sendHeader();
//}

//std::string HttpResponse::getHeader(const std::string& name)
//{
//    if (this->response_headers.find(name) == this->response_headers.end())
//        return "";
//    return this->response_headers.at(name);
//}
//
//std::map<std::string, std::string> HttpResponse::getHeaders()
//{
//    return this->response_headers;
//}

//void HttpResponse::sendData(std::string data)
//{
//    if (this->request->isClosed())
//        return;
//
//    if (!this->header_set)
//        this->sendHeader();
//
//    this->request->getSocket().send(std::move(data));
//}
//
//void HttpResponse::sendData(uint8_t* packet_data, size_t size)
//{
//    if (request->isClosed())
//        return;
//
//    if (!header_set)
//        sendHeader();
//
//    request->getSocket().send(packet_data, size);
//}
//
//void HttpResponse::sendFile(const std::string& file_name, size_t buffer_size)
//{
//    std::ifstream if_stream;
//    if_stream.open(file_name, std::ifstream::in | std::ifstream::binary);
//
//    if (!if_stream.is_open()) {
//        this->setStatus(HttpResponse::HTTP_STATUS_NOT_FOUND, "Not Found");
//        this->addHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/plain");
//        this->sendData("Not Found");
//        this->close();
//        return;
//    }
//
//    this->setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
//
//    uint8_t* packet_data = new uint8_t[buffer_size];
//    while (!if_stream.eof()) {
//        if_stream.read((char*)packet_data, buffer_size);
//        this->sendData(packet_data, if_stream.gcount());
//    }
//
//    delete[] packet_data;
//    if_stream.close();
//    close();
//}
//
//std::string HttpResponse::sendHeader(Socket& socket);
//{
//    if (this->header_set)
//        return;
//
//    std::ostringstream oss;
//    oss << this->request->getVersion() << " " << this->status << " " << this->status_message << lineTerminator;
//
//    for (auto& header : this->response_headers) {
//        oss << header.first.c_str() << ": " << header.second.c_str() << lineTerminator;
//    }
//
//    oss << lineTerminator;
//    this->header_set = true;
//    this->request->getSocket().send(oss.str());
//}

//void HttpResponse::setStatus(const int http_status, const std::string& message)
//{
//    if (this->header_set)
//        return;
//
//    this->status = http_status;
//    this->status_message = message;
//}

HttpResponse::HttpResponse()
    : response_headers()
    , response_status(HttpResponse::HTTP_STATUS_REQUEST_IM_A_TEAPOT)
    , response_body(nullptr)
{
}
HttpResponse::HttpResponse(std::map<std::string, std::string>&& response_headers, HttpStatus status, BoxPtr<ioruntime::IAsyncRead>&& response_body)
    : response_headers(std::move(response_headers))
    , response_status(status)
    , response_body(std::move(response_body))
{
    if (this->response_body.get() == nullptr) {
        // fixme: response_body is BoxPtr which is allocated, memory alloc error can't recover properly
        this->response_body = BoxPtr<DefaultPageBody>::make(status);
    }
}

PollResult<void> HttpResponse::poll_respond(ioruntime::Socket& socket, Waker&& waker)
{
    switch (state) {
    case WriteStatus: {
        if (current.empty()) {
            // fixme: use real http version
            std::stringstream oss;
            oss
                << "HTTP/1.1" //<< this->request->getVersion()
                << " " << std::to_string(this->response_status.code) << " " << this->response_status.message << lineTerminator;
            current = oss.str();
        }

        auto str = current.c_str() + written;
        auto len = current.length() - written;
        auto poll_result = socket.poll_write(str, len, Waker(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();

            if (result < 0) {
                throw std::runtime_error("HttpResponse: poll_respond: status write returned error");
            }
            written += result;

            if (written == (ssize_t)current.length()) {
                current.clear();
                written = 0;
                header_it = response_headers.begin();
                state = WriteHeaders;
                return poll_respond(socket, std::move(waker));
            }
        }
        return PollResult<void>::pending();
    } break;
    case WriteHeaders: {
        if (current.empty()) {
            if (header_it == response_headers.end()) {
                state = WriteBody;
                current = lineTerminator;
                return poll_respond(socket, std::move(waker));
            }
            auto& header = *header_it;
            std::stringstream oss;
            oss << header.first.c_str() << ": " << header.second.c_str() << lineTerminator;
            current = oss.str();
            ++header_it;
        }

        auto str = current.c_str() + written;
        auto len = current.length() - written;
        auto poll_result = socket.poll_write(str, len, Waker(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();

            if (result < 0) {
                throw std::runtime_error("HttpResponse: poll_respond: header write returned error");
            }

            written += result;

            if (written == (ssize_t)current.length()) {
                written = 0;
                current.clear();
                return poll_respond(socket, std::move(waker));
            }
        }
        return PollResult<void>::pending();
    } break;
    case WriteBody: {
        if (response_body.get() == nullptr) {
            // response body is set to null - finish immediately
            return PollResult<void>::ready();
        }
        if (current.empty()) {
            char buffer[1024];
            auto body_poll_result = response_body->poll_read(buffer, sizeof(buffer), Waker(waker));
            if (body_poll_result.is_ready()) {
                auto body_result = body_poll_result.get();
                if (body_result < 0) {
#ifdef DEBUG
                    ERRORPRINT("HttpResponse (future): " << strerror(errno));
#endif
                    throw std::runtime_error("HttpResponse: poll_respond: body read returned error");
                }
                else if (body_result == 0) {
                    // Body read has reached EOF
                    // finish the future
                    return PollResult<void>::ready();
                }
                // create string from const char* and string length
                current = std::string(buffer, body_result);
            } else {
                return PollResult<void>::pending();
            }
        }

        auto str = current.c_str() + written;
        auto len = current.length() - written;
        auto poll_result = socket.poll_write(str, len, Waker(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();

            if (result < 0) {
                throw std::runtime_error("HttpResponse: poll_respond: body write returned error");
            }
            written += result;

            if (written == (ssize_t)current.length()) {
                written = 0;
                current.clear();
                return poll_respond(socket, std::move(waker));
            }
        }
        return PollResult<void>::pending();
    } break;
    default: {
        throw std::runtime_error("HttpResponse: poll_respond: unreachable state");
    } break;
    }
}
HttpResponse::HttpResponse(HttpResponse&& other) noexcept
    : state(other.state)
    , current(std::move(other.current))
    , response_headers(std::move(other.response_headers))
    , header_it(other.header_it)
    , response_status(other.response_status)
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
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::status(HttpStatus status)
{
    this->response_status = status;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::header(const std::string& name, const std::string& value)
{
    this->response_headers.insert(std::pair<std::string, std::string>(name, value));
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::body(BoxPtr<ioruntime::IAsyncRead>&& body)
{
    this->response_body = std::move(body);
    return *this;
}

HttpResponse HttpResponseBuilder::build()
{
    return HttpResponse(std::move(response_headers), response_status, std::move(response_body));
}

HttpResponseBuilder::HttpResponseBuilder():
    response_status(HttpResponse::HTTP_STATUS_REQUEST_IM_A_TEAPOT),
    response_body(nullptr)
{

}

}