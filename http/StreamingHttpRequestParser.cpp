/*
//
// Created by boat on 13-09-20.
//

#include "StreamingHttpRequestParser.hpp"

#include "HttpMethod.hpp"
#include <algorithm>
#include <map>
#include <queue>
#include <utility>
#include <vector>

bool http::StreamingHttpRequestParser::parse(span<uint8_t> incoming)
{
    (void)incoming;
    return false;
}

void http::StreamingHttpRequestParser::next_parser()
{
    switch (tag) {
    case None: {
        new (&request_line_parser) RequestLineParser();
    } break;
    case RequestLine: {
        request_line_parser.~RequestLineParser();
        new (&header_line_parser) HeaderLineParser();
    } break;
    case HeaderLine: {
        header_line_parser.~HeaderLineParser();
        new (&body_parser) BodyParser();
    } break;
    case Body: {
        body_parser.~BodyParser();
    } break;
    }
}


auto http::RequestLineParser::is_uri_valid(std::string_view uri) -> bool
{
    size_t i = 0;
    for (char c : uri) {
        i += 1;
        (void)c;
    }
    return false;
}

auto http::StreamingHttpRequestBuilder::method(http::HttpMethod method) -> http::StreamingHttpRequestBuilder&
{
    _method = method;
    return *this;
}

auto http::StreamingHttpRequestBuilder::uri(string uri) -> http::StreamingHttpRequestBuilder&
{
    _uri = std::move(uri);
    return *this;
}

auto http::StreamingHttpRequestBuilder::version(http::HttpVersion version) -> http::StreamingHttpRequestBuilder&
{
    _version = version;
    return *this;
}

auto http::StreamingHttpRequestBuilder::status(http::HttpStatus status) -> http::StreamingHttpRequestBuilder&
{
    _status = status;
    return *this;
}

auto http::StreamingHttpRequestBuilder::header(string header_name, string header_value) -> http::StreamingHttpRequestBuilder&
{
    _headers.emplace(std::move(header_name), std::move(header_value));
    return *this;
}

auto http::StreamingHttpRequestBuilder::body(vector<uint8_t> body) -> http::StreamingHttpRequestBuilder&
{
    _body = std::move(body);
    return *this;
}

auto http::StreamingHttpRequestBuilder::build() && -> http::StreamingHttpRequest
{
    using std::move;
    return StreamingHttpRequest(
        _method,
        move(_uri),
        _version,
        _status,
        move(_headers),
        move(_body));
}

http::StreamingHttpRequest::StreamingHttpRequest(http::HttpMethod method, string uri, http::HttpVersion version, http::HttpStatus status, map<string, string> headers, vector<uint8_t> body)
    : method(method)
    , uri(std::move(uri))
    , version(version)
    , status(status)
    , headers(std::move(headers))
    , body(std::move(body))
{
}

auto http::StreamingHttpRequest::get_method() -> const http::HttpMethod&
{
    return method;
}

auto http::StreamingHttpRequest::get_uri() -> string const&
{
    return uri;
}

auto http::StreamingHttpRequest::get_version() -> const http::HttpVersion&
{
    return version;
}

auto http::StreamingHttpRequest::get_status() -> const http::HttpStatus&
{
    return status;
}

auto http::StreamingHttpRequest::get_headers() -> map<string, string> const&
{
    return headers;
}

auto http::StreamingHttpRequest::get_body() -> vector<uint8_t> const&
{
    return body;
}
*/
