//
// Created by Djevayo Pattij on 8/22/20.
//

#include "SocketAddr.hpp"
#include <iostream>

namespace net {

SocketAddr::SocketAddr(sockaddr_in addr):
    addr(addr)
{
}

uint32_t SocketAddr::get_ip() const
{
    return addr.sin_addr.s_addr;
}

in_port_t SocketAddr::get_real_port() const
{
    return addr.sin_port;
}

uint16_t SocketAddr::get_port() const
{
    return ntohs(addr.sin_port);
}

}

std::ostream& operator<<(std::ostream& os, const net::SocketAddr& sa)
{
    uint32_t ip_raw = sa.get_ip();
    auto* ip = reinterpret_cast<uint8_t*>(&ip_raw);
    in_port_t port = sa.get_port();

    os << std::to_string(ip[0])
       << "."
       << std::to_string(ip[1])
       << "."
       << std::to_string(ip[2])
       << "."
       << std::to_string(ip[3])
       << ":"
       << std::to_string(port);
    return os;
}