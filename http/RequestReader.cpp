//
// Created by boat on 10/25/20.
//

#include "RequestReader.hpp"

using std::move;
using namespace http::inner_readers;

namespace http {

namespace inner_readers {
    RequestLineReader::RequestLineReader(Method method, Uri uri, Version version)
        : _method(method)
        , _uri(uri)
        , _version(version)
    {
        utils::StringStream ss;

        ss << _method << " " << _uri << " " << _version.version_string << "\r\n";
        _strbuf = ss.str();
        _current = _strbuf;
    }

    auto RequestLineReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
    {
        auto written = buffer.size() >= _current.size() ? _current.size() : buffer.size();
        memcpy(buffer.data(), _current.data(), written);
        _current.remove_prefix(written);
        buffer.remove_prefix_inplace(written);
        waker();
        return PollResult<IoResult>::ready(written);
    }
}

RequestReader::RequestReader(RequestReader&& other) noexcept
    : _request(other._request)
{
    switch (_state) {
        case RequestLineState: {
            new (&_request_line) RequestLineReader(move(other._request_line));
        } break;
        case MessageState: {
            new (&_message) MessageReader(move(other._message));
        } break;
    }
}

RequestReader::RequestReader(RequestReader&& other, OutgoingRequest& request) noexcept
    : _request(request)
{
    switch (_state) {
        case RequestLineState: {
            new (&_request_line) RequestLineReader(move(other._request_line));
        } break;
        case MessageState: {
            new (&_message) MessageReader(move(other._message));
        } break;
    }
}

RequestReader::~RequestReader()
{
    switch (_state) {
        case RequestLineState: {
            _request_line.~RequestLineReader();
        } break;
        case MessageState: {
            _message.~MessageReader();
        } break;
    }
}

RequestReader::RequestReader(OutgoingRequest& request)
    : _request(request)
{
    new (&_request_line) RequestLineReader(_request.get_method(), _request.get_uri(), _request.get_version());
}

auto RequestReader::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_state) {
        case RequestLineState: {
            auto pr = _request_line.poll_read(buffer, Waker(waker));
            if (pr.is_ready()) {
                if (pr.get().is_eof()) {
                    _request_line.~RequestLineReader();
                    _state = MessageState;
                    new (&_message) MessageReader(_request.get_message());
                    return poll_read(buffer, move(waker));
                }
            }
            return pr;
        } break;
        case MessageState: {
            return _message.poll_read(buffer, move(waker));
        } break;
    }
}

}