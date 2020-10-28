//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP
#define WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP

#include "../ioruntime/CharacterStream.hpp"
#include "IncomingMessage.hpp"

namespace http {

class MessageParser : public IFuture<IncomingMessage> {
public:
    class ParserError : public std::runtime_error {
    public:
        explicit ParserError(const char* w);
    };
    class BodyExceededLimit : public ParserError {
    public:
        BodyExceededLimit();
    };
    class GenericExceededBuffer : public ParserError {
    public:
        GenericExceededBuffer();
    };
    class MalformedMessage : public ParserError {
    public:
        MalformedMessage();
    };
    class UnexpectedEof : public ParserError {
    public:
        UnexpectedEof();
    };
    class UndeterminedLength : public ParserError {
    public:
        UndeterminedLength();
    };
    class BadTransferEncoding : public ParserError {
    public:
        BadTransferEncoding();
    };
    explicit MessageParser(ioruntime::CharacterStream& reader, size_t buffer_limit, size_t body_limit, bool has_body);
    MessageParser(MessageParser&& other) noexcept;
    MessageParser(MessageParser&& other, ioruntime::CharacterStream& new_stream);
    MessageParser(MessageParser const&) = delete;
    auto operator=(MessageParser const&) -> MessageParser& = delete;
    auto poll(Waker&& waker) -> PollResult<IncomingMessage> override;

private:
    enum CallState {
        Pending,
        Running,
        Completed,
    };
    auto inner_poll(Waker&& waker) -> CallState;
    enum State {
        HeaderLine,
        // regular body state
        Body,
        // chunked body  states
        ChunkedBodySize,
        ChunkedBodyData,
        // h
        AppendToBody,
    } _state
        = HeaderLine;

    IncomingMessageBuilder _builder;

    ioruntime::CharacterStream& _character_stream;

    size_t _buffer_limit;
    size_t _body_limit;

    bool _has_body;

    optional<size_t> _content_length;
    bool _is_chunk_body = false;

    bool _host_set = false;
    bool _is_http_1_1 = false;

    size_t _last_chunk_size;

    vector<uint8_t> _buffer;
    optional<IncomingBody> _decoded_body;
    optional<ioruntime::SpanReader> _span_reader;
    optional<ioruntime::RefIoCopyFuture> _ricf;

    bool _can_eof = false;
};

}

#endif //WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP
