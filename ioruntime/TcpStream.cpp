//
// Created by Djevayo Pattij on 8/21/20.
//

#include "TcpStream.hpp"

namespace ioruntime {

PollResult<void> ioruntime::TcpStreamResponseFuture::poll(Waker&& waker)
{
    switch (state) {
    case Reading: {
        auto poll_result = socket.poll_read(buffer, BUFFER_SIZE, Waker(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();
            if (result < 0) {
                // Recv returned an error
                throw std::runtime_error("TcpStreamResponseFuture: poll: recv error");
            } else if (result == 0) {
                // Recv returned EOF -> read complete
                state = Writing;
                message = responder(message);
                return poll(std::move(waker));
            } else {
                // Recv returned something
                for (ssize_t idx = 0; idx < result; idx += 1) {
                    message += buffer[idx];
                }
            }
        }
        // The reading state is always pending
        return PollResult<void>::pending();
    } break;
    case Writing: {
        auto const str = message.c_str() + written;
        auto const len = message.length() - written;

        auto poll_result = socket.poll_write(str, len, std::move(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();
            if (result < 0) {
                // Send returned an error
                throw std::runtime_error("TcpStreamResponseFuture: poll: send error");
            } else {
                // Send returned amount of bytes sent,
                // update the "written bytes" counter
                written += result;
                if (written == message.length()) {
                    // We wrote all the bytes,
                    // finish the future.
                    return PollResult<void>::ready();
                }
            }
        }
        return PollResult<void>::pending();
    } break;
    }
}

TcpStream::TcpStream(int fd, SocketAddr address):
    socket(fd),
    address(address)
{
}

TcpStream::TcpStream():
    socket(Socket::uninitialized()),
    address({})
{
}

TcpStream TcpStream::uninitialized()
{
    return TcpStream();
}

TcpStreamResponseFuture::TcpStreamResponseFuture(TcpStream&& stream, std::string (*responder)(std::string&)):
    responder(responder),
    socket(std::move(stream.socket))
{}

TcpStreamResponseFuture TcpStream::respond(std::string (*fp)(std::string&)) &&
{
    return TcpStreamResponseFuture(std::move(*this), fp);
}

SocketAddr const& TcpStream::get_addr() const
{
    return address;
}

}
