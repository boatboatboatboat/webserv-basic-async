//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPREQUEST_HPP
#define WEBSERV_HTTPREQUEST_HPP

#include "../net/TcpStream.hpp"
#include "HttpParser.hpp"
#include "ParserFuture.hpp"
#include <map>
#include <string>
#include <vector>

namespace http {
// forward declarations
class ParserFuture;

// classes
class HttpRequest {
public:
    HttpRequest() = default;
    explicit HttpRequest(std::string const& raw);
    virtual ~HttpRequest();
    static const char HTTP_HEADER_ACCEPT[];
    static const char HTTP_HEADER_ALLOW[];
    static const char HTTP_HEADER_CONNECTION[];
    static const char HTTP_HEADER_CONTENT_LENGTH[];
    static const char HTTP_HEADER_CONTENT_TYPE[];
    static const char HTTP_HEADER_COOKIE[];
    static const char HTTP_HEADER_HOST[];
    static const char HTTP_HEADER_LAST_MODIFIED[];
    static const char HTTP_HEADER_ORIGIN[];
    static const char HTTP_HEADER_SEC_WEBSOCKET_ACCEPT[];
    static const char HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL[];
    static const char HTTP_HEADER_SEC_WEBSOCKET_KEY[];
    static const char HTTP_HEADER_SEC_WEBSOCKET_VERSION[];
    static const char HTTP_HEADER_UPGRADE[];
    static const char HTTP_HEADER_USER_AGENT[];

    static const char HTTP_METHOD_CONNECT[];
    static const char HTTP_METHOD_DELETE[];
    static const char HTTP_METHOD_GET[];
    static const char HTTP_METHOD_HEAD[];
    static const char HTTP_METHOD_OPTIONS[];
    static const char HTTP_METHOD_PATCH[];
    static const char HTTP_METHOD_POST[];
    static const char HTTP_METHOD_PUT[];

    std::string getBody();
    std::string getHeader(const std::string& name);
    std::map<std::string, std::string> getHeaders();
    std::string getMethod();
    std::string getPath();
    std::map<std::string, std::string> getQuery();
    std::string getVersion();
    std::vector<std::string> pathSplit();
    std::string urlDecode(std::string str);
    static ParserFuture parse_async(net::TcpStream& stream, size_t limit = 8192);

private:
    HttpParser parser;
};

}

#endif //WEBSERV_HTTPREQUEST_HPP
