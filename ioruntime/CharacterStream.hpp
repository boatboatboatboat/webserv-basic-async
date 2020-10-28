//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_IORUNTIME_CHARACTERSTREAM_HPP
#define WEBSERV_IORUNTIME_CHARACTERSTREAM_HPP

#include "../futures/IStream.hpp"
#include "IAsyncRead.hpp"

namespace ioruntime {

class CharacterStream: futures::IStream<uint8_t> {
public:
    explicit CharacterStream(IAsyncRead& reader);

    // force reconstruction
    CharacterStream(CharacterStream const&) = delete;
    auto operator=(CharacterStream const&) -> CharacterStream& = delete;
    CharacterStream(CharacterStream&&) noexcept = default;
    auto operator=(CharacterStream&&) noexcept -> CharacterStream&& = delete;

    auto poll_next(Waker&& waker) -> futures::StreamPollResult<uint8_t> override;
    void shift_back(uint8_t c);

private:
    IAsyncRead& _reader;
    uint8_t _buffer[8192];
    size_t _head = 0;
    size_t _max = 0;
    bool _exhausted = true;
    bool _finished = false;
    option::optional<uint8_t> _last_char;
};

}

#endif //WEBSERV_IORUNTIME_CHARACTERSTREAM_HPP
