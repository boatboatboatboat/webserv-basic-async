//
// Created by Djevayo Pattij on 9/6/20.
//

#include "Ipv6Address.hpp"
#include <netinet/in.h>

in6_addr net::Ipv6Address::get_ip_posix() const
{
    return ip;
}

net::Ipv6Address::Ipv6Address(uint64_t b1, uint64_t b2)
{
    auto* x = reinterpret_cast<uint64_t*>(&ip);
    x[0] = b1;
    x[1] = b2;
}
