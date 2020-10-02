//
// Created by boat on 10/1/20.
//

#ifndef WEBSERV_HTTP_HTTPREQUEST_HPP
#define WEBSERV_HTTP_HTTPREQUEST_HPP

#include "HttpMethod.hpp"
#include "HttpVersion.hpp"
#include "HttpHeader.hpp"
#include "Uri.hpp"
#include <vector>
#include <map>

using std::vector;

namespace http {

class HttpRequest;

class HttpRequestBuilder {
public:
    HttpRequestBuilder() = default;
    auto method(HttpMethod method) -> HttpRequestBuilder&;
    auto uri(Uri&& uri) -> HttpRequestBuilder&;
    auto version(HttpVersion version) -> HttpRequestBuilder&;
    auto header(string&& header_name, string&& header_value) -> HttpRequestBuilder&;
    auto body(vector<uint8_t>&& body) -> HttpRequestBuilder&;
    auto build() && -> HttpRequest;

private:
    HttpMethod _method;
    optional<Uri> _uri;
    HttpVersion _version;
    HttpHeaders _headers;
    vector<uint8_t> _body;
};

class HttpRequest {
public:
    HttpRequest(
        HttpMethod method,
        Uri uri,
        HttpVersion version,
        HttpHeaders headers,
        vector<uint8_t> body);
    [[nodiscard]] auto get_method() const -> HttpMethod const&;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> HttpVersion const&;
    [[nodiscard]] auto get_headers() const -> HttpHeaders const&;
    [[nodiscard]] auto get_header(string_view name) const -> optional<string_view>;
    [[nodiscard]] auto get_body() const -> vector<uint8_t> const&;

private:
    HttpMethod _method;
    Uri _uri;
    HttpVersion _version;
    HttpHeaders _headers;
    vector<uint8_t> _body;
};

}

#endif //WEBSERV_HTTP_HTTPREQUEST_HPP
