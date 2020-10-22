//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_NEWREQUEST_HPP
#define WEBSERV_HTTP_NEW_NEWREQUEST_HPP

#include "Message.hpp"

namespace http {

class NewRequest final {
public:
    NewRequest() = delete;
    NewRequest(Method, Uri&&, Version, Message&&);
    NewRequest(NewRequest&&) noexcept = default;
    NewRequest& operator=(NewRequest&&) noexcept = default;
    NewRequest(NewRequest const&) = delete;
    NewRequest& operator=(NewRequest const&) = delete;
    virtual ~NewRequest() = default;

    [[nodiscard]] auto get_method() const -> Method;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> Version;
    [[nodiscard]] auto get_message() -> Message&;

private:
    Method _method;
    Uri _uri;
    Version _version;
    Message _message;
};

class NewRequestBuilder final {
public:
    auto method(Method) -> NewRequestBuilder&;
    auto uri(Uri&&) -> NewRequestBuilder&;
    auto version(Version) -> NewRequestBuilder&;
    auto message(Message&&) -> NewRequestBuilder&;

    auto build() && -> NewRequest;

private:
    optional<Method> _method;
    optional<Uri> _uri;
    optional<Version> _version;
    optional<Message> _message;
};

}

#endif //WEBSERV_HTTP_NEW_NEWREQUEST_HPP
