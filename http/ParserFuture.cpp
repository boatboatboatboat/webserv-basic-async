//
// Created by boat on 8/22/20.
//

#include "ParserFuture.hpp"

namespace http {

ParserFuture::ParserFuture(TcpStream* stream, size_t limit)
    : stream(stream)
    , limit(limit)
{
}

PollResult<HttpRequest> ParserFuture::poll(Waker&& waker)
{
    auto& socket = stream->get_socket();

    char read_buffer[64];

    auto poll_result = socket.poll_read(read_buffer, sizeof(read_buffer), std::move(waker));
    if (poll_result.is_ready()) {
        if (buffer.length() > limit)
            throw std::runtime_error("ParserFuture: poll: buffer over limit");

        auto result = poll_result.get();

        if (result < 0) {
            throw std::runtime_error("ParserFuture: poll: read errored");
        }

        for (ssize_t idx = 0; idx < result; idx += 1)
            buffer += read_buffer[idx];

        if (buffer.ends_with("\r\n"))
            return PollResult<HttpRequest>::ready(HttpRequest(buffer));
    }
    return PollResult<HttpRequest>::pending();
}

}