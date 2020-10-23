//
// Created by boat on 10/23/20.
//

#include "ResponseParser.hpp"

using std::move;

namespace http {

ResponseParser::ResponseParser(ioruntime::CharacterStream& reader, size_t buffer_limit, size_t body_limit)
    : _reader(reader)
    , _buffer_limit(buffer_limit)
    , _body_limit(body_limit)
{
}

ResponseParser::ResponseParser(ResponseParser&& other, ioruntime::CharacterStream& reader) noexcept
    : _state(other._state)
    , _reader(reader)
    , _buffer(move(other._buffer))
    , _message_parser(move(other._message_parser))
    , _buffer_limit(other._buffer_limit)
    , _body_limit(other._body_limit)
{
}

auto ResponseParser::poll(Waker&& waker) -> PollResult<IncomingResponse>
{
    auto call_state = Running;
    while (call_state == Running) {
        call_state = inner_poll(Waker(waker));
    }
    if (call_state == Pending) {
        return PollResult<IncomingResponse>::pending();
    } else {
        return PollResult<IncomingResponse>::ready(move(_builder).build());
    }
}

using futures::StreamPollResult;

auto ResponseParser::inner_poll(Waker&& waker) -> ResponseParser::CallState
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
                case Version: {
                    if (character == ' ') {
                        if (_buffer == http::version::v1_0.version_string) {
                            _builder.version(http::version::v1_0);
                        } else if (_buffer == http::version::v1_1.version_string) {
                            _builder.version(http::version::v1_1);
                        } else {
                            throw std::runtime_error("Unsupported version");
                        }
                        return Running;
                    }
                    _buffer.push_back(character);
                    return Running;
                } break;
                case Status: {
                    if (character == '\n' && !_buffer.empty() && _buffer.back() == '\r') {
                        _buffer.pop_back();
                        auto abcd = _buffer.find(' ');
                        if (abcd == std::string::npos) {
                            throw std::runtime_error("Bad status line");
                        }
                        std::string_view aa = _buffer;
                        auto ax = aa.substr(0, abcd);
                        auto code = utils::string_to_uint64(ax);
                        if (code.has_value()) {
                            _builder.status(http::Status { static_cast<unsigned short>(*code), "ignored" });
                        } else {
                            throw std::runtime_error("Bad status code");
                        }
                    }
                    return Running;
                } break;
                case Message: {
                    _reader.shift_back(character);
                    if (!_message_parser.has_value()) {
                        _message_parser = MessageParser(
                            _reader,
                            _buffer_limit,
                            _body_limit,
                            true);
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
    return Completed;
}

}