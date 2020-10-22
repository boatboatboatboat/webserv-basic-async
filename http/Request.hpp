//
// Created by boat on 10/1/20.
//

#ifndef WEBSERV_HTTP_REQUEST_HPP
#define WEBSERV_HTTP_REQUEST_HPP

#include "Header.hpp"
#include "IncomingBody.hpp"
#include "Method.hpp"
#include "OutgoingBody.hpp"
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
    auto body_incoming(IncomingBody&& body) -> RequestBuilder&;
    auto body_outgoing(OutgoingBody&& body) -> RequestBuilder&;
    auto build() && -> Request;

private:
    Method _method;
    optional<Uri> _uri;
    Version _version;
    Headers _headers;
    optional<IncomingBody> _incoming_body;
    optional<OutgoingBody> _outgoing_body;
};

class Request {
public:
    Request(
        Method method,
        Uri uri,
        Version version,
        Headers headers,
        optional<IncomingBody>&& body);
    Request(
        Method method,
        Uri uri,
        Version version,
        Headers headers,
        optional<OutgoingBody>&& body);
    [[nodiscard]] auto get_method() const -> Method const&;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> Version const&;
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(string_view name) const -> optional<string_view>;
    [[nodiscard]] auto get_body() -> optional<IncomingBody>&;
    [[nodiscard]] auto get_body() const -> optional<IncomingBody> const&;
    [[nodiscard]] auto get_outgoing_body() -> optional<OutgoingBody>&;
    [[nodiscard]] auto get_outgoing_body() const -> optional<OutgoingBody> const&;
    void debug_print() const;

private:
    Method _method;
    Uri _uri;
    Version _version;
    Headers _headers;
    optional<IncomingBody> _incoming_body;
    optional<OutgoingBody> _outgoing_body;
};

}

#endif //WEBSERV_HTTP_REQUEST_HPP
