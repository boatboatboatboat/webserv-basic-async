//
// Created by boat on 10/24/20.
//

#ifndef WEBSERV_HTTP_OUTGOINGMESSAGE_HPP
#define WEBSERV_HTTP_OUTGOINGMESSAGE_HPP

#include "Header.hpp"
#include "OutgoingBody.hpp"

namespace http {

class OutgoingMessage final {
public:
    OutgoingMessage() = delete;
    OutgoingMessage(OutgoingMessage&&) noexcept = default;
    auto operator=(OutgoingMessage&&) noexcept -> OutgoingMessage& = default;
    OutgoingMessage(OutgoingMessage const&) noexcept = delete;
    auto operator=(OutgoingMessage const&) noexcept -> OutgoingMessage& = delete;
    virtual ~OutgoingMessage() = default;

    explicit OutgoingMessage(Headers&& headers);
    OutgoingMessage(Headers&& headers, optional<OutgoingBody>&& body);

    [[nodiscard]] auto get_header(string_view name) const -> optional<string_view>;
    [[nodiscard]] auto get_headers() -> Headers&;
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_body() -> optional<OutgoingBody>&;
    [[nodiscard]] auto get_body() const -> optional<OutgoingBody> const&;

    void drop_body();
private:
    Headers _headers;
    optional<OutgoingBody> _body;
};

class OutgoingMessageBuilder final {
public:
    OutgoingMessageBuilder() = default;
    OutgoingMessageBuilder(OutgoingMessageBuilder&&) noexcept = default;
    auto operator=(OutgoingMessageBuilder&&) noexcept -> OutgoingMessageBuilder& = default;
    OutgoingMessageBuilder(OutgoingMessageBuilder const&) = delete;
    auto operator=(OutgoingMessageBuilder const&) noexcept -> OutgoingMessageBuilder& = delete;
    virtual ~OutgoingMessageBuilder() = default;

    auto header(HeaderName&&, HeaderValue&&) -> OutgoingMessageBuilder&;
    auto headers(Headers&&) -> OutgoingMessageBuilder&;
    auto body(OutgoingBody&&) -> OutgoingMessageBuilder&;

    [[nodiscard]] auto build() && -> OutgoingMessage;

private:
    Headers _headers;
    optional<OutgoingBody> _body;
};

}

#endif //WEBSERV_HTTP_OUTGOINGMESSAGE_HPP
