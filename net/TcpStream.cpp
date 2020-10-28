//
// Created by Djevayo Pattij on 8/21/20.
//

#include "TcpStream.hpp"
#include "Socket.hpp"
#include "SocketAddr.hpp"

namespace net {

TcpStream::TcpStream(int fd, SocketAddr address)
    : _socket(Socket(fd, address))
    , _address(address)
{
}

TcpStream::TcpStream(TcpStream&& other) noexcept
    : _socket(std::move(other._socket))
    , _address(std::move(other._address))
{
}

auto TcpStream::get_addr() const -> SocketAddr const&
{
    return *_address;
}

auto TcpStream::get_socket() -> Socket&
{
    return *_socket;
}

TcpStream::TcpStream(IpAddress address, uint16_t port)
{
    sockaddr addr {};
    utils::bzero(addr);

    if (address.is_v4()) {
        auto* v4addr = reinterpret_cast<sockaddr_in*>(&addr);
        v4addr->sin_addr.s_addr = address.get_v4().get_ip_bytes();
        v4addr->sin_port = htons(port);
        v4addr->sin_family = AF_INET;
    } else if (address.is_v6()) {
        auto* v6addr = reinterpret_cast<sockaddr_in6*>(&addr);
        v6addr->sin6_addr = address.get_v6().get_ip_posix();
        v6addr->sin6_port = htons(port);
        v6addr->sin6_family = AF_INET6;
    } else {
        throw std::logic_error("TcpStream ctor: unreachable");
    }
    int sockfd = socket(addr.sa_family, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("TcpStream: failed to create socket");
    }
    if (connect(sockfd, &addr, sizeof(addr)) != 0) {
        throw std::runtime_error("TcpStream: failed to connect socket");
    }
    _socket = Socket(sockfd, SocketAddr(addr));
    _address = SocketAddr(addr);
}

}
