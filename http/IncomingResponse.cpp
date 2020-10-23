//
// Created by boat on 10/23/20.
//

#include "IncomingResponse.hpp"

using std::move;

namespace http {

auto IncomingResponseBuilder::version(Version version) -> IncomingResponseBuilder&
{
    _version = version;
    return *this;
}

auto IncomingResponseBuilder::status(Status status) -> IncomingResponseBuilder&
{
    _status = status;
    return *this;
}

auto IncomingResponseBuilder::message(IncomingMessage&& message) -> IncomingResponseBuilder&
{
    _message = move(message);
    return *this;
}

auto IncomingResponseBuilder::build() && -> IncomingResponse
{
    return IncomingResponse(*_version, *_status, move(_message.value_or(IncomingMessage(Headers()))));
}

IncomingResponse::IncomingResponse(Version version, Status status, IncomingMessage&& message)
    : _version(version)
    , _status(status)
    , _message(move(message))
{
}

auto IncomingResponse::get_version() const -> Version
{
    return _version;
}

auto IncomingResponse::get_status() const -> Status
{
    return _status;
}

auto IncomingResponse::get_message() -> IncomingMessage&
{
    return _message;
}

auto IncomingResponse::get_message() const -> IncomingMessage const&
{
    return _message;
}

}
