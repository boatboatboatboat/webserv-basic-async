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

using HeaderName = std::string;
using HeaderValue = std::string;

class Header {
public:
    HeaderName name;
    HeaderValue value;
    auto operator==(Header const& other) const -> bool;
};

using Headers = std::vector<Header>;

namespace header {
    // HTTP header names are case insensitive (RFC 2616 Section 4.2.)
    inline HeaderName ACCEPT { "Accept" };
    inline HeaderName ALLOW { "Allow" };
    inline HeaderName AUTHORIZATION { "Authorization" };
    inline HeaderName CONNECTION { "Connection" };
    inline HeaderName CONTENT_LENGTH { "Content-Length" };
    inline HeaderName CONTENT_TYPE { "Content-Type" };
    inline HeaderName COOKIE { "Cookie" };
    inline HeaderName HOST { "Host" };
    inline HeaderName LAST_MODIFIED { "Last-Modified" };
    inline HeaderName ORIGIN { "Origin" };
    inline HeaderName UPGRADE { "Upgrade" };
    inline HeaderName USER_AGENT { "User-Agent" };
    inline HeaderName TRANSFER_ENCODING { "Transfer-Encoding" };
    inline HeaderName SERVER { "Server" };
}

}

#endif //WEBSERV_HTTPHEADER_HPP

#pragma clang diagnostic pop