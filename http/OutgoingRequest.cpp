//
// Created by boat on 10/24/20.
//

#include "OutgoingRequest.hpp"

using std::move;

namespace http {

auto OutgoingRequestBuilder::version(Version version) -> OutgoingRequestBuilder&
{
    _version = version;
    return *this;
}

auto OutgoingRequestBuilder::message(OutgoingMessage&& message) -> OutgoingRequestBuilder&
{
    _message = move(message);
    return *this;
}

auto OutgoingRequestBuilder::build() && -> OutgoingRequest
{
    return OutgoingRequest(*_method, move(*_uri), *_version, move(*_message));
}

auto OutgoingRequestBuilder::method(Method method) -> OutgoingRequestBuilder&
{
    _method = method;
    return *this;
}
auto OutgoingRequestBuilder::uri(Uri&& uri) -> OutgoingRequestBuilder&
{
    _uri = move(uri);
    return *this;
}

OutgoingRequest::OutgoingRequest(Method method, Uri&& uri, Version version, OutgoingMessage&& message)
    : _method(method)
    , _uri(move(uri))
    , _version(version)
    , _message(move(message))
{
}

auto OutgoingRequest::get_version() const -> Version
{
    return _version;
}

auto OutgoingRequest::get_message() -> OutgoingMessage&
{
    return _message;
}

auto OutgoingRequest::get_message() const -> OutgoingMessage const&
{
    return _message;
}

auto OutgoingRequest::get_method() const -> Method
{
    return _method;
}

auto OutgoingRequest::get_uri() const -> Uri const&
{
    return _uri;
}

}