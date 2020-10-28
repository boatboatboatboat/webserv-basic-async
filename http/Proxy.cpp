//
// Created by boat on 10/24/20.
//

#include "Proxy.hpp"

using std::move;

namespace http {

static auto handle_response(IncomingResponse&& response) -> OutgoingResponse
{
    auto headers = move(response.get_message().get_headers());
    {
        auto it = headers.begin();
        while (it != headers.end()) {
            if (utils::str_eq_case_insensitive(it->name, http::header::TRANSFER_ENCODING)) {
                it = headers.erase(it);
                continue;
            }
            if (utils::str_eq_case_insensitive(it->name, http::header::CONTENT_LENGTH)) {
                it = headers.erase(it);
                continue;
            }
            it++;
        }
    }

    OutgoingResponseBuilder orb;

    orb.version(http::version::v1_1)
        .status(response.get_status())
        .headers(move(headers));
    if (response.get_message().get_body().has_value()) {
        orb.body(BoxPtr<IncomingBody>::make(move(*response.get_message().get_body())));
    }
    return move(orb).build();
}

Proxy::Proxy(net::TcpStream&& connection, http::IncomingRequest&& req)
    : _connection(move(connection))
{
    OutgoingMessageBuilder omb;
    omb.headers(move(req.get_message().get_headers())); // lol
    if (req.get_message().get_body().has_value()) {
        omb.body(OutgoingBody(BoxPtr<IncomingBody>::make(move(*req.get_message().get_body()))));
    }

    auto const& uri = req.get_uri();
    utils::StringStream uri_hack;
    uri_hack << "http://" << _connection->get_addr() << uri;
    auto uri_hack2 = uri_hack.str();

    auto out = move(omb).build();

    OutgoingRequestBuilder orb;
    _request = move(orb.method(req.get_method())
                        .uri(Uri(uri_hack2))
                        .version(req.get_version())
                        .message(move(out)))
                   .build();
    _request_reader = RequestReader(*_request);
    _ricf = ioruntime::RefIoCopyFuture(*_request_reader, _connection->get_socket());
}

auto Proxy::poll(Waker&& waker) -> PollResult<OutgoingResponse>
{
    switch (_state) {
        case ForwardClientRequest: {
            auto pr = _ricf->poll(Waker(waker));
            if (pr.is_ready()) {
                _state = ReceiveServerResponse;
                _character_stream = ioruntime::CharacterStream(_connection->get_socket());
                _response_parser = ResponseParser(*_character_stream, 10000000, 10000000);
                return poll(move(waker));
            }
        } break;
        case ReceiveServerResponse: {
            auto pr = _response_parser->poll(Waker(waker));
            if (pr.is_ready()) {
                _outgoing_response = handle_response(move(pr.get()));
                return PollResult<OutgoingResponse>::ready(move(*_outgoing_response));
            }
        } break;
    }
    return PollResult<OutgoingResponse>::pending();
}

Proxy::Proxy(Proxy&& other) noexcept
{
    _connection = move(other._connection);
    _request = move(other._request);
    _outgoing_response = move(other._outgoing_response);
    _request_reader = RequestReader(move(*other._request_reader), *_request);
    _response_parser = move(other._response_parser);
    if (other._character_stream.has_value()) {
        _character_stream = move(other._character_stream);
    }
    if (other._ricf.has_value()) {
        _ricf = ioruntime::RefIoCopyFuture(*_request_reader, _connection->get_socket());
    }
}

}
