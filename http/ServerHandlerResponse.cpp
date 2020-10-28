//
// Created by boat on 10/25/20.
//

#include "ServerHandlerResponse.hpp"

using std::move;

namespace http {

ServerHandlerResponse::ServerHandlerResponse(ServerHandlerResponse&& other) noexcept
{
    switch (other._tag) {
        case ResponseTag: {
            new (&_response) OutgoingResponse(move(other._response));
        } break;
        case ProxyTag: {
            new (&_proxy) Proxy(move(other._proxy));
        } break;
    }
    _tag = other._tag;
}

ServerHandlerResponse::~ServerHandlerResponse()
{
    switch (_tag) {
        case ResponseTag: {
            _response.~OutgoingResponse();
        } break;
        case ProxyTag: {
            _proxy.~Proxy();
        } break;
    }
}

ServerHandlerResponse::ServerHandlerResponse(OutgoingResponse&& response)
{
    _tag = ResponseTag;
    new (&_response) OutgoingResponse(move(response));
}

ServerHandlerResponse::ServerHandlerResponse(Proxy&& proxy)
{
    _tag = ProxyTag;
    new (&_proxy) Proxy(move(proxy));
}

auto ServerHandlerResponse::poll(Waker&& waker) -> PollResult<OutgoingResponse>
{
    if (_tag == ResponseTag) {
        return PollResult<OutgoingResponse>::ready(move(_response));
    } else {
        return _proxy.poll(move(waker));
    }
}

}
