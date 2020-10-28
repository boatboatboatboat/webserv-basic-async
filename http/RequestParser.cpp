//
// Created by boat on 10/22/20.
//

#include "RequestParser.hpp"
#include "Uri.hpp"

using futures::StreamPollResult;
using std::move;

namespace http {

RequestParser::RequestParser(ioruntime::CharacterStream& reader, size_t buffer_limit, size_t body_limit)
    : _reader(reader)
    , _builder()
    , _buffer()
    , _message_parser()
    , _buffer_limit(buffer_limit)
    , _body_limit(body_limit)
{
}

RequestParser::RequestParser(RequestParser&& other, ioruntime::CharacterStream& reader) noexcept
    : _reader(reader)
    , _builder(move(other._builder))
    , _buffer(move(other._buffer))
    , _message_parser(move(other._message_parser))
    , _buffer_limit(other._buffer_limit)
    , _body_limit(other._body_limit)
{
}

// parser helpers

constexpr http::Method VALID_METHODS[8] = {
    http::method::CONNECT,
    http::method::DELETE,
    http::method::GET,
    http::method::HEAD,
    http::method::OPTIONS,
    http::method::PATCH,
    http::method::POST,
    http::method::PUT,
};

inline auto get_method(const string_view possible_method) -> optional<http::Method>
{
    auto methods = span(VALID_METHODS, 8);

    for (auto& method : methods) {
        if (method == possible_method) {
            return optional<http::Method>(method);
        }
    }
    return optional<http::Method>::none();
}

inline auto method_has_body(const string_view possible_method) -> bool
{
    return possible_method == http::method::POST
        || possible_method == http::method::PUT
        || possible_method == http::method::PATCH
        || possible_method == http::method::DELETE;
}

auto RequestParser::poll(Waker&& waker) -> PollResult<IncomingRequest>
{
    auto call_state = Running;
    while (call_state == Running) {
        call_state = inner_poll(Waker(waker));
    }
    if (call_state == Pending) {
        return PollResult<IncomingRequest>::pending();
    } else {
        auto req = move(_builder).build();
        if (req.get_version().version_string == http::version::v1_1.version_string) {
            if (!req.get_message().get_header(http::header::HOST).has_value()) {
                throw MessageParser::MalformedMessage();
            }
        }
        return PollResult<IncomingRequest>::ready(move(req));
    }
}

auto RequestParser::inner_poll(Waker&& waker) -> RequestParser::CallState
{
    if (_buffer.size() > _buffer_limit) {
        throw MessageParser::GenericExceededBuffer();
    }
    auto poll_result = _reader.poll_next(Waker(waker));
    switch (poll_result.get_status()) {
        case StreamPollResult<unsigned char>::Status::Pending: {
            return Pending;
        } break;
        case StreamPollResult<unsigned char>::Status::Ready: {
            auto character = poll_result.get();
            switch (_state) {
                case Method: {
                    if (character == ' ') {
                        auto method = get_method(_buffer);
                        if (method.has_value()) {
                            _method = *method;
                            _builder.method(*method);
                            _state = Uri;
                            _buffer.clear();
                            return Running;
                        } else {
                            throw InvalidMethod();
                        }
                    }
                    _buffer.push_back(character);
                    return Running;
                } break;
                case Uri: {
                    if (character == ' ') {
                        _builder.uri(http::Uri(_buffer));
                        _state = Version;
                        _buffer.clear();
                        return Running;
                    }
                    if (_buffer.size() >= _buffer_limit) {
                        throw UriExceededBuffer();
                    }
                    _buffer.push_back(character);
                    return Running;
                } break;
                case Version: {
                    if (character == '\n' && !_buffer.empty() && _buffer.back() == '\r') {
                        _buffer.pop_back();
                        if (_buffer == http::version::v1_0.version_string) {
                            _builder.version(http::version::v1_0);
                        } else if (_buffer == http::version::v1_1.version_string) {
                            _builder.version(http::version::v1_1);
                        } else {
                            throw UnsupportedVersion();
                        }
                        _state = Message;
                        return Running;
                    }
                    _buffer.push_back(character);
                    return Running;
                } break;
                case Message: {
                    _reader.shift_back(character);
                    if (!_message_parser.has_value()) {
                        _message_parser = MessageParser(
                            _reader,
                            _buffer_limit,
                            _body_limit,
                            method_has_body(_method));
                    }
                    auto res = _message_parser->poll(move(waker));
                    if (res.is_pending()) {
                        return Pending;
                    } else {
                        _builder.message(move(res.get()));
                        return Completed;
                    }
                } break;
            }
        } break;
        case StreamPollResult<unsigned char>::Status::Finished: {
            throw MessageParser::UnexpectedEof();
        } break;
    }
}

RequestParser::InvalidMethod::InvalidMethod()
    : MessageParser::ParserError("Invalid HTTP method")
{
}

RequestParser::UnsupportedVersion::UnsupportedVersion()
    : MessageParser::ParserError("Unsupported HTTP version")
{
}

}
