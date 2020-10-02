//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_TCPSTREAM_HPP
#define WEBSERV_IORUNTIME_TCPSTREAM_HPP

#include "../futures/PollResult.hpp"
#include "Socket.hpp"
#include "SocketAddr.hpp"
#include <sys/socket.h>

using futures::IFuture;
using futures::PollResult;

namespace net {

class TcpStream {
public:
    TcpStream() = delete;
    explicit TcpStream(int fd, SocketAddr address);
    TcpStream(TcpStream&& other) noexcept;
    ~TcpStream() = default;

    [[nodiscard]] auto get_addr() const -> SocketAddr const&;
    auto get_socket() -> Socket&;

private:
    Socket _socket;
    SocketAddr _address;
};
}

#endif //WEBSERV_IORUNTIME_TCPSTREAM_HPP
