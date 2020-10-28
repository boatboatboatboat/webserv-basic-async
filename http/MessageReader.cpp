//
// Created by boat on 10/24/20.
//

#include "MessageReader.hpp"

using std::move;
using namespace http::inner_readers;

namespace http {

namespace inner_readers {
    HeadersReader::HeadersReader(Headers const& headers)
        : _headers(headers)
    {
        _header_it = _headers.begin();
        if (_header_it != _headers.end()) {
            _current = _header_it->name;
        } else {
            _state = Clrf;
            _current = "";
            --_header_it; // lmao
        }
    }

    auto HeadersReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
    {
        switch (_state) {
            case Name: {
                if (_current.empty()) {
                    _state = Splitter;
                    _current = ": ";
                }
            } break;
            case Splitter: {
                if (_current.empty()) {
                    _state = Value;
                    _current = _header_it->value;
                }
            } break;
            case Value: {
                if (_current.empty()) {
                    _state = Clrf;
                    _current = CLRF;
                }
            } break;
            case Clrf: {
                if (_current.empty()) {
                    // current depleted, select next header
                    ++_header_it;
                    if (_header_it == _headers.end()) {
                        return PollResult<IoResult>::ready(IoResult::eof());
                    }
                    _current = _header_it->name;
                    _state = Name;
                }
            } break;
        }
        auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
        memcpy(buffer.data(), _current.data(), written);
        _current.remove_prefix(written);
        waker();
        return PollResult<IoResult>::ready(written);
    }

    BodyReader::BodyReader(OutgoingBody& body)
        : _body(body)
    {
    }

    auto BodyReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
    {
        return _body.poll_read(buffer, move(waker));
    }

    ChunkedBodyReader::ChunkedBodyReader(OutgoingBody& body)
        : _state(Clrf2)
        , _current(_data_buf, 0)
        , _current_body(_data_buf, 0)
        , _body(body)
    {
    }

    auto ChunkedBodyReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
    {
        switch (_state) {
            case ChunkSize: {
                // write the chunk size
                if (_current.empty()) {
                    _current = span((uint8_t*)CLRF.data(), CLRF.size());
                    _state = Clrf1;
                    return poll_read(buffer, move(waker));
                }
            } break;
            case Clrf1: {
                // write the CLRF after the chunk size
                if (_current.empty()) {
                    _current = _current_body;
                    _state = ChunkData;
                    return poll_read(buffer, move(waker));
                }
            } break;
            case ChunkData: {
                // write the actual chunk data
                if (_current.empty()) {
                    _current = span((uint8_t*)CLRF.data(), CLRF.size());
                    _state = Clrf2;
                    return poll_read(buffer, move(waker));
                }
            } break;
            case Clrf2: {
                // write the CLRF after the chunk data
                if (_current.empty()) {
                    auto poll_res = _body.poll_read(span(_data_buf, sizeof(_data_buf)), Waker(waker));
                    if (poll_res.is_ready()) {
                        auto res = poll_res.get();
                        if (res.is_eof()) {
                            _current = span((uint8_t*)"0\r\n\r\n", 5);
                            _state = Eof;
                        } else if (res.is_error()) {
                            return PollResult<IoResult>::ready(IoResult::error());
                        } else if (res.is_success()) {
                            _current_body = span(_data_buf, res.get_bytes_read());
                            auto b = utils::uint64_to_hexstring(res.get_bytes_read());
                            for (size_t c = 0; c < b.size(); c += 1) {
                                _num[c] = b[c];
                            }
                            _current = span((uint8_t*)_num, b.size());
                            _state = ChunkSize;
                        }
                        return poll_read(buffer, move(waker));
                    } else {
                        return PollResult<IoResult>::pending();
                    }
                }
            } break;
            case Eof: {
                // write 0\r\n\r\n
                if (_current.empty()) {
                    return PollResult<IoResult>::ready(IoResult::eof());
                }
            } break;
        }
        auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
        memcpy(buffer.data(), _current.data(), written);
        _current.remove_prefix_inplace(written);
        waker();
        return PollResult<IoResult>::ready(written);
    }

    auto HeaderTerminatorReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
    {
        auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
        memcpy(buffer.data(), _current.data(), written);
        _current.remove_prefix(written);
        waker();
        return PollResult<IoResult>::ready(written);
    }

}

auto MessageReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case HeaderState: {
            auto poll_res = _header.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _header.~HeadersReader();
                    new (&_header_terminator) HeaderTerminatorReader();
                    _state = HeaderTerminatorState;
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case HeaderTerminatorState: {
            auto poll_res = _header_terminator.poll_read(buffer, Waker(waker));
            if (poll_res.is_ready()) {
                if (poll_res.get().is_eof()) {
                    // Clean up old state
                    _header_terminator.~HeaderTerminatorReader();
                    // Change to Body state
                    if (!_message.get_body().has_value()) {
                        // We don't have a body - just terminate
                        return PollResult<IoResult>::ready(IoResult::eof());
                    } else if (_message.get_header(http::header::CONTENT_LENGTH).has_value()) {
                        // Content-Length means regular body
                        _state = BodyState;
                        new (&_body) BodyReader(*_message.get_body());
                    } else {
                        // No content-length means chunked transfer
                        _state = ChunkedBodyState;
                        new (&_chunked_body) ChunkedBodyReader(*_message.get_body());
                    }
                    return PollResult<IoResult>::pending();
                }
            }
            return poll_res;
        } break;
        case BodyState: {
            return _body.poll_read(buffer, move(waker));
        } break;
        case ChunkedBodyState: {
            return _chunked_body.poll_read(buffer, move(waker));
        } break;
    }
}

MessageReader::MessageReader(OutgoingMessage& message)
    : _message(message)
    , _state(HeaderState)
{
    new (&_header) HeadersReader(message.get_headers());
}

MessageReader::MessageReader(MessageReader&& other) noexcept
    : _message(other._message)
    , _state(other._state)
{
    switch (_state) {
        case HeaderState: {
            new (&_header) HeadersReader(move(other._header));
        } break;
        case HeaderTerminatorState: {
            new (&_header_terminator) HeaderTerminatorReader(move(other._header_terminator));
        } break;
        case BodyState: {
            new (&_body) BodyReader(move(other._body));
        } break;
        case ChunkedBodyState: {
            new (&_chunked_body) ChunkedBodyReader(move(other._chunked_body));
        } break;
    }
}

MessageReader::~MessageReader()
{
    switch (_state) {
        case HeaderState: {
            _header.~HeadersReader();
        } break;
        case HeaderTerminatorState: {
            _header_terminator.~HeaderTerminatorReader();
        } break;
        case BodyState: {
            _body.~BodyReader();
        } break;
        case ChunkedBodyState: {
            _chunked_body.~ChunkedBodyReader();
        } break;
    }
}

}