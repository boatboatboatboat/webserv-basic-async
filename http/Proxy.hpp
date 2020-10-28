//
// Created by boat on 10/24/20.
//

#ifndef WEBSERV_HTTP_PROXY_HPP
#define WEBSERV_HTTP_PROXY_HPP

#include "../net/TcpStream.hpp"
#include "IncomingRequest.hpp"
#include "OutgoingRequest.hpp"
#include "RequestReader.hpp"
#include "ResponseParser.hpp"

namespace http {

class Proxy final : public IFuture<OutgoingResponse> {
public:
    Proxy() = delete;
    Proxy(net::TcpStream&& connection, http::IncomingRequest&& req);
    Proxy(Proxy&&) noexcept;
    auto operator=(Proxy&&) noexcept -> Proxy& = delete;
    Proxy(Proxy const&) noexcept = delete;
    auto operator=(Proxy const&) noexcept -> Proxy& = delete;
    auto poll(Waker&& waker) -> PollResult<OutgoingResponse> override;
    ~Proxy() override = default;

private:
    enum State {
        ForwardClientRequest,
        ReceiveServerResponse,
    } _state
        = ForwardClientRequest;

    optional<net::TcpStream> _connection;
    optional<OutgoingRequest> _request;
    optional<OutgoingResponse> _outgoing_response;
    optional<RequestReader> _request_reader;
    optional<ResponseParser> _response_parser;
    optional<ioruntime::CharacterStream> _character_stream;
    optional<ioruntime::RefIoCopyFuture> _ricf;
};

}

#endif //WEBSERV_HTTP_PROXY_HPP
