//
// Created by boat on 30-08-20.
//

#include "Ipv4Address.hpp"

uint32_t net::Ipv4Address::get_ip_bytes() const
{
    return ip;
}

net::Ipv4Address::Ipv4Address(uint32_t ip)
    : ip(ip)
{
}
