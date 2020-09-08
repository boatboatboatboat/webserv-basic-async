//
// Created by Djevayo Pattij on 9/6/20.
//

#include "Ipv6Address.hpp"
#include "../utils/mem_zero.hpp"
#include <iostream>
#include <libnet.h>
#include <netinet/in.h>

auto net::Ipv6Address::get_ip_posix() const -> in6_addr
{
    return ip;
}

net::Ipv6Address::Ipv6Address(uint64_t b1, uint64_t b2)
{
    auto* x = reinterpret_cast<uint64_t*>(&ip);
    x[0] = b1;
    x[1] = b2;
}

net::Ipv6Address::Ipv6Address(string_view str)
{
    // FIXME: pton
    std::string x(str);
    int r = inet_pton(AF_INET6, x.c_str(), &ip);
    if (r != 1) {
        throw std::runtime_error("bad ip address");
    }
}

auto operator<<(std::ostream& os, const net::Ipv6Address& sa) -> std::ostream&
{
    in6_addr ip_raw = sa.get_ip_posix();
    auto* ip = reinterpret_cast<uint16_t*>(&ip_raw);
    os << std::hex;
    size_t idx = 0;
    bool pdouble = false;
    while (idx < 8) {
        if (ip[idx] == 0 && idx != 7 && ip[idx + 1] == 0) {
            while (ip[idx] == 0 && idx < 8)
                idx += 1;
            if (pdouble)
                os << ":";
            else
                os << "::";
            pdouble = false;
            continue ;
        } else {
            os << __builtin_bswap16(ip[idx]);
        }
        if (idx < 7) {
            pdouble = true;
            os << ":";
        }
        idx += 1;
    }
    return os << std::dec;
}