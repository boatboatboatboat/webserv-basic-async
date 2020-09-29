//
// Created by boat on 13-09-20.
//

#include "StreamingHttpRequestParser.hpp"

namespace http {

auto StreamingHttpRequestBuilder::method(HttpMethod method) -> StreamingHttpRequestBuilder&
{
    _method = method;
    return *this;
}

auto StreamingHttpRequestBuilder::uri(Uri&& uri) -> StreamingHttpRequestBuilder&
{
    _uri = std::move(uri);
    return *this;
}

auto StreamingHttpRequestBuilder::version(HttpVersion version) -> StreamingHttpRequestBuilder&
{
    _version = version;
    return *this;
}

auto StreamingHttpRequestBuilder::header(string&& header_name, string&& header_value) -> StreamingHttpRequestBuilder&
{
    _headers.emplace(header_name, header_value);
    return *this;
}

auto StreamingHttpRequestBuilder::body(vector<uint8_t>&& body) -> StreamingHttpRequestBuilder&
{
    _body = std::move(body);
    return *this;
}

auto StreamingHttpRequestBuilder::build() && -> StreamingHttpRequest
{
    if (!_uri.has_value()) {
        throw std::runtime_error("no uri set in builder");
    }
    return StreamingHttpRequest(
        _method,
        std::move(*_uri),
        _version,
        std::move(_headers),
        std::move(_body));
}

StreamingHttpRequest::StreamingHttpRequest(HttpMethod method, Uri uri, HttpVersion version, map<string, string> headers, vector<uint8_t> body)
    : _method(method)
    , _uri(std::move(uri))
    , _version(version)
    , _headers(std::move(headers))
    , _body(std::move(body))
{
}

auto StreamingHttpRequest::get_method() const -> HttpMethod const&
{
    return _method;
}

auto StreamingHttpRequest::get_uri() const -> Uri const&
{
    return _uri;
}

auto StreamingHttpRequest::get_version() const -> HttpVersion const&
{
    return _version;
}

auto StreamingHttpRequest::get_headers() const -> map<string, string> const&
{
    return _headers;
}

auto StreamingHttpRequest::get_body() const -> vector<uint8_t> const&
{
    return _body;
}

auto StreamingHttpRequest::get_header(string_view name) const -> optional<string_view>
{
    auto const& val = std::find_if(_headers.begin(), _headers.end(), [=](auto& field_pair) { return name == field_pair.first; });
    if (val == _headers.end()) {
        return option::nullopt;
    } else {
        return optional<string_view>(val->second);
    }
}

}