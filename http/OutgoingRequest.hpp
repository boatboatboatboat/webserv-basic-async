//
// Created by boat on 10/24/20.
//

#ifndef WEBSERV_HTTP_OUTGOINGREQUEST_HPP
#define WEBSERV_HTTP_OUTGOINGREQUEST_HPP

#include "OutgoingMessage.hpp"
#include "Status.hpp"
#include "Version.hpp"

namespace http {

class OutgoingRequest final {
public:
    OutgoingRequest() = delete;
    OutgoingRequest(OutgoingRequest&&) noexcept = default;
    auto operator=(OutgoingRequest&&) noexcept -> OutgoingRequest& = default;
    OutgoingRequest(OutgoingRequest const&) noexcept = delete;
    auto operator=(OutgoingRequest const&) noexcept -> OutgoingRequest& = delete;
    virtual ~OutgoingRequest() = default;

    OutgoingRequest(Method, Uri&&, Version, OutgoingMessage&&);

    [[nodiscard]] auto get_method() const -> Method;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> Version;
    [[nodiscard]] auto get_message() -> OutgoingMessage&;
    [[nodiscard]] auto get_message() const -> OutgoingMessage const&;

private:
    Method _method;
    Uri _uri;
    Version _version;
    OutgoingMessage _message;
};

class OutgoingRequestBuilder final {
public:
    OutgoingRequestBuilder() = default;
    OutgoingRequestBuilder(OutgoingRequestBuilder&&) noexcept = default;
    auto operator=(OutgoingRequestBuilder&&) noexcept -> OutgoingRequestBuilder& = default;
    OutgoingRequestBuilder(OutgoingRequestBuilder const&) noexcept = delete;
    auto operator=(OutgoingRequestBuilder const&) noexcept -> OutgoingRequestBuilder& = delete;
    virtual ~OutgoingRequestBuilder() = default;

    auto method(Method) -> OutgoingRequestBuilder&;
    auto uri(Uri&&) -> OutgoingRequestBuilder&;
    auto version(Version) -> OutgoingRequestBuilder&;
    auto message(OutgoingMessage&&) -> OutgoingRequestBuilder&;

    [[nodiscard]] auto build() && -> OutgoingRequest;

private:
    optional<Method> _method;
    optional<Uri> _uri;
    optional<Version> _version;
    optional<OutgoingMessage> _message;
};

}

#endif //WEBSERV_HTTP_OUTGOINGREQUEST_HPP
