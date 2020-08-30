//
// Created by boat on 8/27/20.
//

#ifndef WEBSERV_HTTPHEADER_HPP
#define WEBSERV_HTTPHEADER_HPP

#include <string>


namespace http {

using HttpHeaderName = std::string_view;
using HttpHeaderValue = std::string_view;

class HttpHeader {
public:
    HttpHeaderName name;
    HttpHeaderValue value;
    bool operator==(HttpHeader const& other) const;
};

namespace header {
    // HTTP header names are case insensitive (RFC 2616 Section 4.2.)

    inline constexpr HttpHeaderName ACCEPT { "Accept" };
    inline constexpr HttpHeaderName ALLOW { "Allow" };
    inline constexpr HttpHeaderName CONNECTION { "Connection" };
    inline constexpr HttpHeaderName CONTENT_LENGTH { "Content-Length" };
    inline constexpr HttpHeaderName CONTENT_TYPE { "Content-Type" };
    inline constexpr HttpHeaderName COOKIE { "Cookie" };
    inline constexpr HttpHeaderName HOST { "Host" };
    inline constexpr HttpHeaderName LAST_MODIFIED { "Last-Modified" };
    inline constexpr HttpHeaderName ORIGIN { "Origin" };
    inline constexpr HttpHeaderName UPGRADE { "Upgrade" };
    inline constexpr HttpHeaderName USER_AGENT { "User-Agent" };
}

}

#endif //WEBSERV_HTTPHEADER_HPP
