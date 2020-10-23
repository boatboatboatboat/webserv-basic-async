//
// Created by boat on 10/22/20.
//

#ifndef WEBSERV_HTTP_NEW_NEWREQUESTPARSER_HPP
#define WEBSERV_HTTP_NEW_NEWREQUESTPARSER_HPP

#include "../ioruntime/CharacterStream.hpp"
#include "IncomingRequest.hpp"
#include "MessageParser.hpp"

namespace http {

class RequestParser final : public IFuture<IncomingRequest> {
public:
    class InvalidMethod : MessageParser::ParserError {
    public:
        InvalidMethod();
    };
    class UnsupportedVersion : MessageParser::ParserError {
    public:
        UnsupportedVersion();
    };
    class UriExceededBuffer : MessageParser::ParserError {
    public:
        inline UriExceededBuffer()
            : MessageParser::ParserError("Uri exceeded buffer")
        {
        }
    };

public:
    RequestParser() = delete;
    RequestParser(ioruntime::CharacterStream& reader, size_t buffer_limit, size_t body_limit);
    RequestParser(RequestParser&& other, ioruntime::CharacterStream& reader) noexcept;
    RequestParser(RequestParser&&) noexcept = default;
    auto operator=(RequestParser&&) noexcept -> RequestParser& = delete;
    RequestParser(RequestParser const&) = delete;
    auto operator=(RequestParser const&) = delete;
    ~RequestParser() final = default;

    auto poll(Waker&& waker) -> PollResult<IncomingRequest> override;

private:
    enum CallState {
        Pending,
        Running,
        Completed,
    };

    auto inner_poll(Waker&& waker) -> CallState;
    enum State {
        Method,
        Uri,
        Version,
        Message,
    } _state
        = Method;

    ioruntime::CharacterStream& _reader;
    IncomingRequestBuilder _builder;
    std::string _buffer;
    http::Method _method;
    optional<MessageParser> _message_parser;

    size_t _buffer_limit;
    size_t _body_limit;
};

}

#endif //WEBSERV_HTTP_NEW_NEWREQUESTPARSER_HPP
