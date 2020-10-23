//
// Created by boat on 10/23/20.
//

#ifndef WEBSERV_HTTP_RESPONSEPARSER_HPP
#define WEBSERV_HTTP_RESPONSEPARSER_HPP

#include "IncomingResponse.hpp"
#include "OutgoingResponse.hpp"

namespace http {

class ResponseParser final : IFuture<IncomingResponse> {
public:
    ResponseParser() = delete;
    explicit ResponseParser(ioruntime::CharacterStream& _reader, size_t buffer_limit, size_t body_limit);
    ResponseParser(ResponseParser&&) noexcept = default;
    ResponseParser(ResponseParser&& other, ioruntime::CharacterStream& reader) noexcept;
    auto operator=(ResponseParser&&) noexcept -> ResponseParser& = delete;
    ResponseParser(ResponseParser const&) = delete;
    auto operator=(ResponseParser const&) -> ResponseParser& = delete;
    ~ResponseParser() override = default;

    auto poll(Waker&&) -> PollResult<IncomingResponse> override;

private:
    enum CallState {
        Pending,
        Running,
        Completed,
    };
    auto inner_poll(Waker&&) -> CallState;
    enum State {
        Version,
        Status,
        Message,
    } _state
        = Version;

    ioruntime::CharacterStream& _reader;
    IncomingResponseBuilder _builder;
    std::string _buffer;
    optional<MessageParser> _message_parser;

    size_t _buffer_limit;
    size_t _body_limit;
};

}
#endif //WEBSERV_HTTP_RESPONSEPARSER_HPP
