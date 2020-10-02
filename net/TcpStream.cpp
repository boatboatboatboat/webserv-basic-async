//
// Created by Djevayo Pattij on 8/21/20.
//

#include "TcpStream.hpp"
#include "Socket.hpp"
#include "SocketAddr.hpp"

namespace net {

TcpStream::TcpStream(int fd, SocketAddr address)
    : _socket(fd, address)
    , _address(address)
{
}

TcpStream::TcpStream(TcpStream&& other) noexcept
    : _socket(std::move(other._socket))
    , _address(other._address)
{
}

auto TcpStream::get_addr() const -> SocketAddr const&
{
    return _address;
}

auto TcpStream::get_socket() -> Socket&
{
    return _socket;
}

}
