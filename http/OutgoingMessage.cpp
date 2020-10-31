//
// Created by boat on 10/24/20.
//

#include "OutgoingMessage.hpp"

using std::move;

namespace http {

auto OutgoingMessageBuilder::header(HeaderName&& name, HeaderValue&& value) -> OutgoingMessageBuilder&
{
    _headers.emplace_back(Header { move(name), move(value) });
    return *this;
}

auto OutgoingMessageBuilder::headers(Headers&& headers) -> OutgoingMessageBuilder&
{
    _headers.insert(_headers.end(), headers.begin(), headers.end());
    return *this;
}

auto OutgoingMessageBuilder::body(OutgoingBody&& body) -> OutgoingMessageBuilder&
{
    _body = move(body);
    return *this;
}

auto OutgoingMessageBuilder::build() && -> OutgoingMessage
{
    return OutgoingMessage(move(_headers), move(_body));
}

OutgoingMessage::OutgoingMessage(Headers&& headers)
    : _headers(move(headers))
    , _body(option::nullopt)
{
}

OutgoingMessage::OutgoingMessage(Headers&& headers, optional<OutgoingBody>&& body)
    : _headers(move(headers))
    , _body(move(body))
{
}

auto OutgoingMessage::get_headers() -> Headers&
{
    return _headers;
}

auto OutgoingMessage::get_headers() const -> Headers const&
{
    return _headers;
}

auto OutgoingMessage::get_body() -> optional<OutgoingBody>&
{
    return _body;
}

auto OutgoingMessage::get_body() const -> optional<OutgoingBody> const&
{
    return _body;
}

auto OutgoingMessage::get_header(string_view name) const -> optional<string_view>
{
    for (auto const& header_a : _headers) {
        if (utils::str_eq_case_insensitive(header_a.name, name)) {
            return string_view(header_a.value);
        }
    }
    return option::nullopt;
}

void OutgoingMessage::drop_body()
{
    _body.reset();
}

}
