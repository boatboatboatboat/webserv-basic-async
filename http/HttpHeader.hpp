#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by boat on 8/27/20.
//

#ifndef WEBSERV_HTTPHEADER_HPP
#define WEBSERV_HTTPHEADER_HPP

#include <string>
#include <vector>

namespace http {

using HttpHeaderName = std::string;
using HttpHeaderValue = std::string;

class HttpHeader {
public:
    HttpHeaderName name;
    HttpHeaderValue value;
    auto operator==(HttpHeader const& other) const -> bool;
};

using HttpHeaders = std::vector<HttpHeader>;

namespace header {
    // HTTP header names are case insensitive (RFC 2616 Section 4.2.)
    inline HttpHeaderName ACCEPT { "Accept" };
    inline HttpHeaderName ALLOW { "Allow" };
    inline HttpHeaderName CONNECTION { "Connection" };
    inline HttpHeaderName CONTENT_LENGTH { "Content-Length" };
    inline HttpHeaderName CONTENT_TYPE { "Content-Type" };
    inline HttpHeaderName COOKIE { "Cookie" };
    inline HttpHeaderName HOST { "Host" };
    inline HttpHeaderName LAST_MODIFIED { "Last-Modified" };
    inline HttpHeaderName ORIGIN { "Origin" };
    inline HttpHeaderName UPGRADE { "Upgrade" };
    inline HttpHeaderName USER_AGENT { "User-Agent" };
    inline HttpHeaderName TRANSFER_ENCODING { "Transfer-Encoding" };
}

}

#endif //WEBSERV_HTTPHEADER_HPP

#pragma clang diagnostic pop