//
// Created by boat on 10/1/20.
//

#include "Request.hpp"
#include "../utils/utils.hpp"
#include <algorithm>

namespace http {

auto RequestBuilder::method(Method method) -> RequestBuilder&
{
    _method = method;
    return *this;
}

auto RequestBuilder::uri(Uri&& uri) -> RequestBuilder&
{
    _uri = std::move(uri);
    return *this;
}

auto RequestBuilder::version(Version version) -> RequestBuilder&
{
    _version = version;
    return *this;
}

auto RequestBuilder::header(string&& header_name, string&& header_value) -> RequestBuilder&
{
    _headers.push_back(Header { header_name, header_value });
    return *this;
}

auto RequestBuilder::body(vector<uint8_t>&& body) -> RequestBuilder&
{
    _body = std::move(body);
    return *this;
}

auto RequestBuilder::build() && -> Request
{
    if (!_uri.has_value()) {
        throw std::runtime_error("no uri set in builder");
    }
    return Request(
        _method,
        std::move(*_uri),
        _version,
        std::move(_headers),
        std::move(_body));
}

Request::Request(Method method, Uri uri, Version version, Headers headers, vector<uint8_t> body)
    : _method(method)
    , _uri(std::move(uri))
    , _version(version)
    , _headers(std::move(headers))
    , _body(std::move(body))
{
}

auto Request::get_method() const -> Method const&
{
    return _method;
}

auto Request::get_uri() const -> Uri const&
{
    return _uri;
}

auto Request::get_version() const -> Version const&
{
    return _version;
}

auto Request::get_headers() const -> Headers const&
{
    return _headers;
}

auto Request::get_body() const -> vector<uint8_t> const&
{
    return _body;
}

auto Request::get_header(string_view name) const -> optional<string_view>
{
    for (auto& header : _headers) {
        if (utils::str_eq_case_insensitive(name, header.name)) {
            return optional<string_view>(header.value);
        }
    }
    return option::nullopt;
}

}