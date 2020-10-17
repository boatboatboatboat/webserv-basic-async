//
// Created by boat on 10/1/20.
//

#ifndef WEBSERV_HTTP_REQUEST_HPP
#define WEBSERV_HTTP_REQUEST_HPP

#include "Header.hpp"
#include "Method.hpp"
#include "RequestBody.hpp"
#include "Uri.hpp"
#include "Version.hpp"
#include <map>
#include <vector>

using std::vector;

namespace http {

class Request;

class RequestBuilder {
public:
    RequestBuilder() = default;
    auto method(Method method) -> RequestBuilder&;
    auto uri(Uri&& uri) -> RequestBuilder&;
    auto version(Version version) -> RequestBuilder&;
    auto header(string&& header_name, string&& header_value) -> RequestBuilder&;
    auto body(RequestBody&& body) -> RequestBuilder&;
    auto build() && -> Request;

private:
    Method _method;
    optional<Uri> _uri;
    Version _version;
    Headers _headers;
    optional<RequestBody> _body;
};

class Request {
public:
    Request(
        Method method,
        Uri uri,
        Version version,
        Headers headers,
        optional<RequestBody> body);
    [[nodiscard]] auto get_method() const -> Method const&;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> Version const&;
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(string_view name) const -> optional<string_view>;
    [[nodiscard]] auto get_body() -> optional<RequestBody>&;
    [[nodiscard]] auto get_body() const -> optional<RequestBody> const&;
    void debug_print() const;

private:
    Method _method;
    Uri _uri;
    Version _version;
    Headers _headers;
    optional<RequestBody> _body;
};

}

#endif //WEBSERV_HTTP_REQUEST_HPP
