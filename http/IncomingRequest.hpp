//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_NEWREQUEST_HPP
#define WEBSERV_HTTP_NEW_NEWREQUEST_HPP

#include "IncomingMessage.hpp"
#include "Method.hpp"
#include "Uri.hpp"
#include "Version.hpp"

namespace http {

class IncomingRequest final {
public:
    IncomingRequest() = delete;
    IncomingRequest(Method, Uri&&, Version, IncomingMessage&&);
    IncomingRequest(IncomingRequest&&) noexcept = default;
    auto operator=(IncomingRequest&&) noexcept -> IncomingRequest& = default;
    IncomingRequest(IncomingRequest const&) = delete;
    auto operator=(IncomingRequest const&) -> IncomingRequest& = delete;
    virtual ~IncomingRequest() = default;

    [[nodiscard]] auto get_method() const -> Method;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> Version;
    [[nodiscard]] auto get_message() -> IncomingMessage&;
    [[nodiscard]] auto get_message() const -> IncomingMessage const&;

private:
    Method _method;
    Uri _uri;
    Version _version;
    IncomingMessage _message;
};

class IncomingRequestBuilder final {
public:
    IncomingRequestBuilder() = default;
    IncomingRequestBuilder(IncomingRequestBuilder&&) noexcept = default;
    auto operator=(IncomingRequestBuilder&&) noexcept -> IncomingRequestBuilder& = default;
    IncomingRequestBuilder(IncomingRequestBuilder const&) = delete;
    auto operator=(IncomingRequestBuilder const&) noexcept -> IncomingRequestBuilder& = delete;
    virtual ~IncomingRequestBuilder() = default;

    auto method(Method) -> IncomingRequestBuilder&;
    auto uri(Uri&&) -> IncomingRequestBuilder&;
    auto version(Version) -> IncomingRequestBuilder&;
    auto message(IncomingMessage&&) -> IncomingRequestBuilder&;

    auto build() && -> IncomingRequest;

private:
    optional<Method> _method;
    optional<Uri> _uri;
    optional<Version> _version;
    optional<IncomingMessage> _message;
};

}

#endif //WEBSERV_HTTP_NEW_NEWREQUEST_HPP
