//
// Created by boat on 13-09-20.
//

#include "StreamingHttpRequestParser.hpp"
#include "HttpMethod.hpp"

bool http::StreamingHttpRequestParser::parse(char* buffer, size_t size)
{
    return false;
}

constexpr std::array<http::HttpMethod, 8> VALID_METHODS = {
    http::method::CONNECT,
    http::method::DELETE,
    http::method::GET,
    http::method::HEAD,
    http::method::OPTIONS,
    http::method::PATCH,
    http::method::POST,
    http::method::PUT,
};

auto http::StreamingHttpRequestParser::RequestLineParser::is_method_valid(std::string_view method) -> bool
{
    return std::any_of(VALID_METHODS.begin(), VALID_METHODS.end(), [&](auto real) { return real.starts_with(method); });
}

auto http::StreamingHttpRequestParser::RequestLineParser::is_uri_valid(std::string_view uri) -> bool
{
    size_t i = 0;
    for (char c : uri) {

        i += 1;
    }
    return false;
}
