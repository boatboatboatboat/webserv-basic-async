//
// Created by boat on 19-10-20.
//

#include "Message.hpp"

using std::move;

namespace http {

Message::Message(http::Headers&& headers)
    : _headers(move(headers))
{
}

Message::Message(Headers&& headers, IncomingBody&& body)
    : _headers(move(headers))
    , _incoming_body(move(body))
{
}

Message::Message(Headers&& headers, OutgoingBody&& body)
    : _headers(move(headers))
    , _outgoing_body(move(body))
{
}

auto Message::get_headers() -> Headers&
{
    return _headers;
}

auto Message::get_headers() const -> Headers const&
{
    return _headers;
}

auto Message::get_header(string_view header) const -> optional<string_view>
{
    for (auto const& header_a : _headers) {
        if (utils::str_eq_case_insensitive(header_a.name, header)) {
            return string_view(header_a.name);
        }
    }
    return option::nullopt;
}

auto Message::get_incoming_body() -> optional<IncomingBody>&
{
    return _incoming_body;
}

auto Message::get_incoming_body() const -> optional<IncomingBody> const&
{
    return _incoming_body;
}

auto Message::get_outgoing_body() -> optional<OutgoingBody>&
{
    return _outgoing_body;
}

auto Message::get_outgoing_body() const -> optional<OutgoingBody> const&
{
    return _outgoing_body;
}

auto MessageBuilder::header(HeaderName name, HeaderValue value) -> MessageBuilder&
{
    _headers.emplace_back(Header { move(name), move(value) });
    return *this;
}

auto MessageBuilder::headers(Headers&& headers) -> MessageBuilder&
{
    _headers.insert(_headers.end(), headers.begin(), headers.end());
    return *this;
}

auto MessageBuilder::incoming_body(IncomingBody&& body) -> MessageBuilder&
{
    _incoming_body = move(body);
    return *this;
}

auto MessageBuilder::outgoing_body(OutgoingBody&& body) -> MessageBuilder&
{
    _outgoing_body = move(body);
    return *this;
}

auto MessageBuilder::build() && -> Message
{
    if (_incoming_body.has_value()) {
        return Message(move(_headers), move(*_incoming_body));
    } else if (_outgoing_body.has_value()) {
        return Message(move(_headers), move(*_outgoing_body));
    } else {
        return Message(move(_headers));
    }
}

}