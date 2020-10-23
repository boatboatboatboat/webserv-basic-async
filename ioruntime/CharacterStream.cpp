//
// Created by boat on 19-10-20.
//

#include "CharacterStream.hpp"

using futures::StreamPollResult;
using std::move;

namespace ioruntime {

CharacterStream::CharacterStream(IAsyncRead& reader)
    : _reader(reader)
{
}

auto CharacterStream::poll_next(Waker&& waker) -> StreamPollResult<uint8_t>
{
    if (_last_char.has_value()) {
        uint8_t lc = *_last_char;
        _last_char.reset();
        return StreamPollResult<uint8_t>::ready(uint8_t(lc));
    }
    if (_exhausted) {
        auto poll_result = _reader.poll_read(span(_buffer, sizeof(_buffer)), move(waker));

        if (poll_result.is_ready()) {
            _exhausted = false;
            _head = 0;
            auto result = poll_result.get();
            if (result.is_error()) {
                throw std::runtime_error("CharacterStream: poll_next: read returned negative");
            }
            _max = result.get_bytes_read();
        } else {
            return StreamPollResult<uint8_t>::pending();
        }
    }
    if (_max == 0)
        return StreamPollResult<uint8_t>::finished();
    uint8_t c = _buffer[_head];
    _head += 1;
    _exhausted = _head == _max;
    return StreamPollResult<uint8_t>::ready(uint8_t(c));
}

void CharacterStream::shift_back(uint8_t c)
{
    _last_char = c;
}

}
