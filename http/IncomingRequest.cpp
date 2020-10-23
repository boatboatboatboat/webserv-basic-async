//
// Created by boat on 19-10-20.
//

#include "IncomingRequest.hpp"

#include <utility>

using std::move;

namespace http {

IncomingRequest::IncomingRequest(Method method, Uri&& uri, Version version, IncomingMessage&& message)
    : _method(method)
    , _uri(move(uri))
    , _version(version)
    , _message(move(message))
{
}

auto IncomingRequest::get_method() const -> Method
{
    return _method;
}

auto IncomingRequest::get_uri() const -> Uri const&
{
    return _uri;
}

auto IncomingRequest::get_version() const -> Version
{
    return _version;
}

auto IncomingRequest::get_message() -> IncomingMessage&
{
    return _message;
}

auto IncomingRequest::get_message() const -> IncomingMessage const&
{
    return _message;
}

auto IncomingRequestBuilder::method(Method method) -> IncomingRequestBuilder&
{
    _method = method;
    return *this;
}

auto IncomingRequestBuilder::uri(Uri&& uri) -> IncomingRequestBuilder&
{
    _uri = move(uri);
    return *this;
}

auto IncomingRequestBuilder::version(Version version) -> IncomingRequestBuilder&
{
    _version = version;
    return *this;
}

auto IncomingRequestBuilder::build() && -> IncomingRequest
{
    return IncomingRequest(
        *_method,
        move(*_uri),
        *_version,
        move(_message.value_or(IncomingMessage(Headers()))));
}

auto IncomingRequestBuilder::message(IncomingMessage&& message) -> IncomingRequestBuilder&
{
    _message = move(message);
    return *this;
}

}
