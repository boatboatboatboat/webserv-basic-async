//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_MESSAGE_HPP
#define WEBSERV_HTTP_NEW_MESSAGE_HPP

#include "../Header.hpp"
#include "../IncomingBody.hpp"
#include "../OutgoingBody.hpp"

namespace http {

class Message final {
public:
    Message() = delete;
    explicit Message(Headers&& headers);
    Message(Headers&& headers, IncomingBody&& body);
    Message(Headers&& headers, OutgoingBody&& body);
    Message(Message&&) noexcept = default;
    Message& operator=(Message&&) noexcept = default;
    Message(Message const&) = delete;
    Message& operator=(Message const&) = delete;
    virtual ~Message() = default;

    [[nodiscard]] auto get_headers() -> Headers&;
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(string_view header) const -> optional<string_view>;
    [[nodiscard]] auto get_incoming_body() -> optional<IncomingBody>&;
    [[nodiscard]] auto get_incoming_body() const -> optional<IncomingBody> const&;
    [[nodiscard]] auto get_outgoing_body() -> optional<OutgoingBody>&;
    [[nodiscard]] auto get_outgoing_body() const -> optional<OutgoingBody> const&;

private:
    Headers _headers;
    optional<IncomingBody> _incoming_body;
    optional<OutgoingBody> _outgoing_body;
};

class MessageBuilder final {
public:
    MessageBuilder() = default;
    MessageBuilder(MessageBuilder&&) noexcept = default;
    MessageBuilder& operator=(MessageBuilder&&) noexcept = default;
    MessageBuilder(MessageBuilder const&) noexcept = delete;
    MessageBuilder& operator=(MessageBuilder const&) noexcept = delete;
    virtual ~MessageBuilder() = default;

    auto header(HeaderName, HeaderValue) -> MessageBuilder&;
    auto headers(Headers&&) -> MessageBuilder&;
    auto incoming_body(IncomingBody&&) -> MessageBuilder&;
    auto outgoing_body(OutgoingBody&&) -> MessageBuilder&;

    auto build() && -> Message;

private:
    Headers _headers;
    optional<IncomingBody> _incoming_body;
    optional<OutgoingBody> _outgoing_body;
};

}

#endif //WEBSERV_HTTP_NEW_MESSAGE_HPP
