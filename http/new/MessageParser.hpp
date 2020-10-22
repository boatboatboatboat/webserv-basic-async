//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP
#define WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP

#include "../../ioruntime/CharacterStream.hpp"
#include "Message.hpp"

namespace http {

class MessageParser : public IFuture<Message> {
public:
    class ParserError : public std::runtime_error {
    public:
        explicit ParserError(const char* w);
    };
    class RequestUriExceededBuffer : public ParserError {
    public:
        RequestUriExceededBuffer();
    };
    class BodyExceededLimit : public ParserError {
    public:
        BodyExceededLimit();
    };
    class GenericExceededBuffer : public ParserError {
    public:
        GenericExceededBuffer();
    };
    class InvalidMethod : public ParserError {
    public:
        InvalidMethod();
    };
    class InvalidVersion : public ParserError {
    public:
        InvalidVersion();
    };
    class MalformedRequest : public ParserError {
    public:
        MalformedRequest();
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
    explicit MessageParser(ioruntime::CharacterStream& reader, size_t buffer_limit, size_t body_limit);
    MessageParser(MessageParser&& other) noexcept;
    MessageParser(MessageParser&& other, IAsyncRead& new_stream);
    auto poll(Waker&& waker) -> PollResult<Message> override;

private:
    enum CallState {
        Pending,
        Running,
        Completed,
    };
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

    ioruntime::CharacterStream& _reader;

    size_t _buffer_limit;
    size_t _body_limit;

    optional<size_t> _content_length;
    bool _is_chunk_body = false;

    bool _host_set = false;
    bool _is_http_1_1 = false;

    vector<uint8_t> _buffer;
    optional<IncomingBody> _decoded_body;
    optional<ioruntime::SpanReader> _span_reader;
    optional<ioruntime::RefIoCopyFuture> _ricf;
};

}

#endif //WEBSERV_HTTP_NEW_MESSAGEPARSER_HPP
