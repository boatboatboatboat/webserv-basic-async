#include "HttpRequest.hpp"
#include "../net/TcpStream.hpp"
#include "../utils/utils.hpp"
#include "HttpResponse.hpp"
#include <algorithm>
#include <sstream>
#include <vector>

#define STATE_NAME 0
#define STATE_VALUE 1

namespace http {

HttpRequest::HttpRequest(std::string const& raw)
{
    this->parser.parse(raw);
    std::vector<std::string> parts = utils::split(getHeader(http::header::CONNECTION), ',');
}

HttpRequest::~HttpRequest() = default;

/**
 * @brief Get the body of the HttpRequest.
 */
std::string HttpRequest::getBody()
{
    return this->parser.getBody();
}

/**
 * @brief Get the named header.
 * @param [in] name The name of the header field to retrieve.
 * @return The value of the header field.
 */
std::string HttpRequest::getHeader(std::string_view name)
{
    return this->parser.getHeader(name);
}

std::map<std::string, std::string> HttpRequest::getHeaders()
{
    return this->parser.getHeaders();
}

std::string HttpRequest::getMethod()
{
    return this->parser.getMethod();
}

std::string HttpRequest::getPath()
{
    return this->parser.getURL();
}

/**
 * @brief Get the query part of the request.
 * The query is a set of name = value pairs.  The return is a map keyed by the name items.
 *
 * @return The query part of the request.
 */
std::map<std::string, std::string> HttpRequest::getQuery()
{
    std::map<std::string, std::string> query_map;
    std::string possible_query_string = this->getPath();
    int qindex = possible_query_string.find_first_of('?');

    if (qindex < 0)
        return query_map;

    std::string query_string = possible_query_string.substr(qindex + 1, -1);

    int state = STATE_NAME;
    std::string name;
    std::string value;

    for (char currentChar : query_string) {
        if (state == STATE_NAME) {
            if (currentChar != '=') {
                name += currentChar;
            } else {
                state = STATE_VALUE;
                value = "";
            }
        } else {
            if (currentChar != '&') {
                value += currentChar;
            } else {
                query_map[name] = value;
                state = STATE_NAME;
                name = "";
            }
        }
    }

    if (state == STATE_VALUE)
        query_map[name] = value;

    return query_map;
}

std::string HttpRequest::getVersion()
{
    return this->parser.getVersion();
}

/**
 * Return the constituent parts of the path.
 * If we imagine a path as composed of parts separated by slashes, then this function
 * returns a vector composed of the parts.  For example:
 *
 * @return A vector of the constituent parts of the path.
 */
std::vector<std::string> HttpRequest::pathSplit()
{
    std::istringstream stream(this->getPath());
    std::vector<std::string> ret;
    std::string pathPart;

    while (std::getline(stream, pathPart, '/')) {
        ret.push_back(pathPart);
    }

    return ret;
}

/**
 * A simple hex conversion function.
 * @param ch
 * @return
 */
inline char from_hex(char ch)
{
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/**
 * Decode a URL
 * @param text the text to decode
 * @return the decoded string.
 */
std::string HttpRequest::urlDecode(std::string text)
{
    char h;
    std::ostringstream escaped;
    escaped.fill('0');

    for (auto i = text.begin(), n = text.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        if (c == '%') {
            if (i[1] && i[2]) {
                h = from_hex(i[1]) << 4 | from_hex(i[2]);
                escaped << h;
                i += 2;
            }
        } else if (c == '+') {
            escaped << ' ';
        } else {
            escaped << c;
        }
    }

    return escaped.str();
}

ParserFuture HttpRequest::parse_async(net::TcpStream& stream, size_t limit)
{
    return ParserFuture(&stream, limit);
}

}