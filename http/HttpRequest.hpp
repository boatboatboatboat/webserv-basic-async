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
    std::string getBody();
    std::string getHeader(std::string_view name);
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
