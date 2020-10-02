//
// Created by boat on 10/1/20.
//

#include "HttpRequest.hpp"
#include "../utils/utils.hpp"
#include <algorithm>

namespace http {

auto HttpRequestBuilder::method(HttpMethod method) -> HttpRequestBuilder&
{
    _method = method;
    return *this;
}

auto HttpRequestBuilder::uri(Uri&& uri) -> HttpRequestBuilder&
{
    _uri = std::move(uri);
    return *this;
}

auto HttpRequestBuilder::version(HttpVersion version) -> HttpRequestBuilder&
{
    _version = version;
    return *this;
}

auto HttpRequestBuilder::header(string&& header_name, string&& header_value) -> HttpRequestBuilder&
{
    _headers.push_back(HttpHeader { header_name, header_value });
    return *this;
}

auto HttpRequestBuilder::body(vector<uint8_t>&& body) -> HttpRequestBuilder&
{
    _body = std::move(body);
    return *this;
}

auto HttpRequestBuilder::build() && -> HttpRequest
{
    if (!_uri.has_value()) {
        throw std::runtime_error("no uri set in builder");
    }
    return HttpRequest(
        _method,
        std::move(*_uri),
        _version,
        std::move(_headers),
        std::move(_body));
}

HttpRequest::HttpRequest(HttpMethod method, Uri uri, HttpVersion version, HttpHeaders headers, vector<uint8_t> body)
    : _method(method)
    , _uri(std::move(uri))
    , _version(version)
    , _headers(std::move(headers))
    , _body(std::move(body))
{
}

auto HttpRequest::get_method() const -> HttpMethod const&
{
    return _method;
}

auto HttpRequest::get_uri() const -> Uri const&
{
    return _uri;
}

auto HttpRequest::get_version() const -> HttpVersion const&
{
    return _version;
}

auto HttpRequest::get_headers() const -> HttpHeaders const&
{
    return _headers;
}

auto HttpRequest::get_body() const -> vector<uint8_t> const&
{
    return _body;
}

auto HttpRequest::get_header(string_view name) const -> optional<string_view>
{
    for (auto& header : _headers) {
        if (utils::str_eq_case_insensitive(name, header.name)) {
            return optional<string_view>(header.value);
        }
    }
    return option::nullopt;
}

}