//
// Created by boat on 19-10-20.
//

#include "NewRequest.hpp"

#include <utility>

using std::move;

namespace http {

NewRequest::NewRequest(Method method, Uri&& uri, Version version, Message&& message)
    : _method(method)
    , _uri(move(uri))
    , _version(version)
    , _message(move(message))
{
}

auto NewRequest::get_method() const -> Method
{
    return _method;
}

auto NewRequest::get_uri() const -> Uri const&
{
    return _uri;
}

auto NewRequest::get_version() const -> Version
{
    return _version;
}

auto NewRequest::get_message() -> Message&
{
    return _message;
}

auto NewRequestBuilder::method(Method method) -> NewRequestBuilder&
{
    _method = method;
    return *this;
}

auto NewRequestBuilder::uri(Uri&& uri) -> NewRequestBuilder&
{
    _uri = move(uri);
    return *this;
}

auto NewRequestBuilder::version(Version version) -> NewRequestBuilder&
{
    _version = version;
    return *this;
}

auto NewRequestBuilder::build() && -> NewRequest
{
    return NewRequest(
        *_method,
        move(*_uri),
        *_version,
        move(*_message));
}

auto NewRequestBuilder::message(Message&& message) -> NewRequestBuilder&
{
    _message = move(message);
    return *this;
}

}
