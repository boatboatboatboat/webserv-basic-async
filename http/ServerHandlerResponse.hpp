//
// Created by boat on 10/25/20.
//

#ifndef WEBSERV_HTTP_SERVERHANDLERRESPONSE_HPP
#define WEBSERV_HTTP_SERVERHANDLERRESPONSE_HPP

#include "OutgoingResponse.hpp"
#include "Proxy.hpp"

namespace http {

class ServerHandlerResponse final : IFuture<OutgoingResponse> {
public:
    ServerHandlerResponse() = delete;
    ServerHandlerResponse(ServerHandlerResponse&&) noexcept;
    auto operator=(ServerHandlerResponse&&) noexcept -> ServerHandlerResponse& = delete;
    ServerHandlerResponse(ServerHandlerResponse const&) = delete;
    auto operator=(ServerHandlerResponse const&) -> ServerHandlerResponse& = delete;
    ~ServerHandlerResponse() override;

    ServerHandlerResponse(OutgoingResponse&& response);
    ServerHandlerResponse(Proxy&& proxy);

    auto poll(Waker&&) -> PollResult<OutgoingResponse> override;

private:
    enum Tag {
        ResponseTag,
        ProxyTag,
    } _tag;
    union {
        OutgoingResponse _response;
        Proxy _proxy;
    };
};

}

#endif //WEBSERV_HTTP_SERVERHANDLERRESPONSE_HPP
