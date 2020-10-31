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
    inline HeaderName ACCEPT_CHARSET { "Accept-Charset" };
    inline HeaderName ACCEPT_LANGUAGE { "Accept-Language" };
    inline HeaderName ALLOW { "Allow" };
    inline HeaderName AUTHORIZATION { "Authorization" };
    inline HeaderName CONNECTION { "Connection" };
    inline HeaderName CONTENT_ENCODING { "Content-Encoding" };
    inline HeaderName CONTENT_LANGUAGE { "Content-Language" };
    inline HeaderName CONTENT_LENGTH { "Content-Length" };
    inline HeaderName CONTENT_LOCATION { "Content-Location" };
    inline HeaderName CONTENT_TYPE { "Content-Type" };
    inline HeaderName COOKIE { "Cookie" };
    inline HeaderName DATE { "Date" };
    inline HeaderName HOST { "Host" };
    inline HeaderName LAST_MODIFIED { "Last-Modified" };
    inline HeaderName LOCATION { "Location" };
    inline HeaderName ORIGIN { "Origin" };
    inline HeaderName REFERER { "Referer" };
    inline HeaderName SERVER { "Server" };
    inline HeaderName TRANSFER_ENCODING { "Transfer-Encoding" };
    inline HeaderName UPGRADE { "Upgrade" };
    inline HeaderName USER_AGENT { "User-Agent" };
    inline HeaderName WWW_AUTHENTICATE { "WWW-Authenticate" };
}

}

#endif //WEBSERV_HTTPHEADER_HPP
