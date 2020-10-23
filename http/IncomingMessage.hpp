//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_INCOMINGMESSAGE_HPP
#define WEBSERV_HTTP_NEW_INCOMINGMESSAGE_HPP

#include "Header.hpp"
#include "IncomingBody.hpp"

namespace http {

class IncomingMessage final {
public:
    IncomingMessage() = delete;
    explicit IncomingMessage(Headers&& headers);
    IncomingMessage(Headers&& headers, IncomingBody&& body);
    IncomingMessage(IncomingMessage&&) noexcept = default;
    auto operator=(IncomingMessage&&) noexcept -> IncomingMessage& = default;
    IncomingMessage(IncomingMessage const&) = delete;
    auto operator=(IncomingMessage const&) -> IncomingMessage& = delete;
    virtual ~IncomingMessage() = default;

    [[nodiscard]] auto get_headers() -> Headers&;
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(std::string_view header) const -> optional<std::string_view>;
    [[nodiscard]] auto get_body() -> optional<IncomingBody>&;
    [[nodiscard]] auto get_body() const -> optional<IncomingBody> const&;

private:
    Headers _headers;
    optional<IncomingBody> _body;
};

class IncomingMessageBuilder final {
public:
    IncomingMessageBuilder() = default;
    IncomingMessageBuilder(IncomingMessageBuilder&&) noexcept = default;
    auto operator=(IncomingMessageBuilder&&) noexcept -> IncomingMessageBuilder& = default;
    IncomingMessageBuilder(IncomingMessageBuilder const&) noexcept = delete;
    auto operator=(IncomingMessageBuilder const&) noexcept -> IncomingMessageBuilder& = delete;
    virtual ~IncomingMessageBuilder() = default;

    auto header(HeaderName, HeaderValue) -> IncomingMessageBuilder&;
    auto headers(Headers&&) -> IncomingMessageBuilder&;
    auto body(IncomingBody&&) -> IncomingMessageBuilder&;

    auto build() && -> IncomingMessage;

private:
    Headers _headers;
    optional<IncomingBody> _body;
};

}

#endif //WEBSERV_HTTP_NEW_INCOMINGMESSAGE_HPP
