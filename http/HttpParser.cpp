#include "HttpParser.hpp"
#include "../utils/utils.hpp"
#include "HttpRequest.hpp"
#include <cstdlib>
#include <string>

namespace http {

static std::string lineTerminator = "\r\n";

static std::string toStringToken(std::string::iterator& it, std::string& str, std::string& token)
{
    std::string ret;
    std::string part;
    auto itToken = token.begin();

    for (; it != str.end(); ++it) {
        if ((*it) == (*itToken)) {
            part += (*itToken);
            ++itToken;
            if (itToken == token.end()) {
                ++it;
                break;
            }
        } else {
            if (part.empty()) {
                ret += part;
                part.clear();
                itToken = token.begin();
            }

            ret += *it;
        }
    }

    return ret;
}

static std::string toCharToken(std::string::iterator& it, std::string& str, char token)
{
    std::string ret;
    for (; it != str.end(); ++it) {
        if ((*it) == token) {
            ++it;
            break;
        }
        ret += *it;
    }

    return ret;
}

std::pair<std::string, std::string> parseHeader(std::string& line)
{
    auto it = line.begin();
    std::string name = toCharToken(it, line, ':');
    utils::toLower(name);
    auto value = utils::trim(toStringToken(it, line, lineTerminator));
    return std::pair<std::string, std::string>(name, value);
}

HttpParser::HttpParser() = default;
HttpParser::~HttpParser() = default;

std::string HttpParser::getBody() { return this->body; }
std::string HttpParser::getMethod() { return this->method; }
std::string HttpParser::getURL() { return this->url; }
std::string HttpParser::getVersion() { return this->version; }
std::string HttpParser::getStatus() { return this->status; }
std::string HttpParser::getReason() { return this->reason; }

std::map<std::string, std::string> HttpParser::getHeaders()
{
    return this->headers;
}

std::string HttpParser::getHeader(const std::string& name)
{
    std::string local_name = name;

    utils::toLower(local_name);
    if (!hasHeader(local_name))
        return "";

    return this->headers.at(local_name);
}

bool HttpParser::hasHeader(const std::string& name)
{
    std::string local_name = name;
    return this->headers.find(utils::toLower(local_name)) != this->headers.end();
}

//void HttpParser::parse(std::string line)
//{
//    line = socket.readToDelimiter(lineTerminator);
//    this->parseRequestLine(line);
//    line = socket.readToDelimiter(lineTerminator);
//
//    while (!line.empty()) {
//        this->headers.insert(parseHeader(line));
//        line = socket.readToDelimiter(lineTerminator);
//    }
//
//    if (getMethod() != "POST" && getMethod() != "PUT")
//        return;
//
//    if (hasHeader(HttpRequest::HTTP_HEADER_CONTENT_LENGTH)) {
//        auto val = this->getHeader(HttpRequest::HTTP_HEADER_CONTENT_LENGTH);
//        auto length = std::atoi(val.c_str());
//        uint8_t data[length];
//        socket.receive(data, length, true);
//        this->body = std::string((char*)data, length);
//
//    } else {
//        uint8_t data[512];
//        auto rc = socket.receive(data, sizeof(data), false);
//
//        if (rc > 0)
//            this->body = std::string((char*)data, rc);
//    }
//}

void HttpParser::parse(std::string message)
{
    auto it = message.begin();
    auto line = toStringToken(it, message, lineTerminator);

    this->parseRequestLine(line);
    line = toStringToken(it, message, lineTerminator);

    while (!line.empty()) {
        this->headers.insert(parseHeader(line));
        line = toStringToken(it, message, lineTerminator);
    }

    this->body = message.substr(std::distance(message.begin(), it));
}

void HttpParser::parseRequestLine(std::string& line)
{
    auto it = line.begin();

    this->method = toCharToken(it, line, ' ');
    this->url = toCharToken(it, line, ' ');
    this->version = toCharToken(it, line, ' ');
}

void HttpParser::parseResponse(std::string message)
{
    auto it = message.begin();
    auto line = toStringToken(it, message, lineTerminator);
    this->parseStatusLine(line);
    line = toStringToken(it, message, lineTerminator);

    while (!line.empty()) {
        this->headers.insert(parseHeader(line));
        line = toStringToken(it, message, lineTerminator);
    }

    this->body = message.substr(std::distance(message.begin(), it));
}

void HttpParser::parseStatusLine(std::string& line)
{
    auto it = line.begin();

    this->version = toCharToken(it, line, ' ');
    this->status = toCharToken(it, line, ' ');
    this->reason = toStringToken(it, line, lineTerminator);
}



}