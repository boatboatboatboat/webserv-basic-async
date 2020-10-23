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
    (void)address;
    (void)port;
    if (address.is_v4()) {
        sockaddr_in addr {};
        (void)addr;
    } else if (address.is_v6()) {
    }
    throw std::logic_error("TcpStream ctor: unreachable");
}

}
