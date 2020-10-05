//
// Created by Djevayo Pattij on 8/22/20.
//

#include "SocketAddr.hpp"
#include <iostream>

namespace net {

SocketAddr::SocketAddr(sockaddr_in addr)
    : tag(IpV4)
    , addr_v4(addr)
{
}

SocketAddr::SocketAddr(sockaddr_in6 addr)
    : tag(IpV6)
    , addr_v6(addr)
{
}

auto SocketAddr::get_v4() const -> uint32_t
{
    return addr_v4.sin_addr.s_addr;
}

auto SocketAddr::get_real_port() const -> in_port_t
{
    if (tag == IpV4) {
        return addr_v4.sin_port;
    } else if (tag == IpV6) {
        return addr_v6.sin6_port;
    } else {
        throw std::logic_error("unreachable");
    }
}

auto SocketAddr::get_port() const -> uint16_t
{
    return ntohs(get_real_port());
}
auto SocketAddr::get_addr() const -> sockaddr*
{
    return const_cast<sockaddr*>(reinterpret_cast<sockaddr const*>(&addr_v4));
}

auto SocketAddr::is_v4() const -> bool
{
    return tag == IpV4;
}

auto SocketAddr::is_v6() const -> bool
{
    return tag == IpV6;
}

auto SocketAddr::get_v6() const -> in6_addr
{
    return addr_v6.sin6_addr;
}

}

auto operator<<(std::ostream& os, const net::SocketAddr& sa) -> std::ostream&
{
    if (sa.is_v4()) {
        uint32_t ip_raw = sa.get_v4();
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
    } else if (sa.is_v6()) {
        in6_addr ip_raw = sa.get_v6();
        auto* ip = reinterpret_cast<uint16_t*>(&ip_raw);
        os << "[" << std::hex;
        size_t idx = 0;
        bool pdouble = false;
        while (idx < 8) {
            if (ip[idx] == 0 && idx != 7 && ip[idx + 1] == 0) {
                while (idx < 8 && ip[idx] == 0)
                    idx += 1;
                if (pdouble)
                    os << ":";
                else
                    os << "::";
                pdouble = false;
                continue;
            } else {
                os << __builtin_bswap16(ip[idx]);
            }
            if (idx < 7) {
                pdouble = true;
                os << ":";
            }
            idx += 1;
        }
        os << "]:" << std::to_string(sa.get_port());
        os << std::dec;
    }
    return os;
}