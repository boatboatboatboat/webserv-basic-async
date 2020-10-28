//
// Created by boat on 19-10-20.
//

#include "IncomingMessage.hpp"

using std::move;
using std::string_view;

namespace http {

IncomingMessage::IncomingMessage(http::Headers&& headers)
    : _headers(move(headers))
{
}

IncomingMessage::IncomingMessage(Headers&& headers, IncomingBody&& body)
    : _headers(move(headers))
    , _body(move(body))
{
}

auto IncomingMessage::get_headers() -> Headers&
{
    return _headers;
}

auto IncomingMessage::get_headers() const -> Headers const&
{
    return _headers;
}

auto IncomingMessage::get_header(string_view header) const -> optional<std::string_view>
{
    for (auto const& header_a : _headers) {
        if (utils::str_eq_case_insensitive(header_a.name, header)) {
            return header_a.value;
        }
    }
    return option::nullopt;
}

auto IncomingMessage::get_body() -> optional<IncomingBody>&
{
    return _body;
}

auto IncomingMessage::get_body() const -> optional<IncomingBody> const&
{
    return _body;
}

auto IncomingMessageBuilder::header(HeaderName&& name, HeaderValue&& value) -> IncomingMessageBuilder&
{
    _headers.emplace_back(Header { move(name), move(value) });
    return *this;
}

auto IncomingMessageBuilder::headers(Headers&& headers) -> IncomingMessageBuilder&
{
    _headers.insert(_headers.end(), headers.begin(), headers.end());
    return *this;
}

auto IncomingMessageBuilder::body(IncomingBody&& p_body) -> IncomingMessageBuilder&
{
    _body = move(p_body);
    return *this;
}

auto IncomingMessageBuilder::build() && -> IncomingMessage
{
    if (_body.has_value()) {
        return IncomingMessage(move(_headers), move(*_body));
    } else {
        return IncomingMessage(move(_headers));
    }
}

}