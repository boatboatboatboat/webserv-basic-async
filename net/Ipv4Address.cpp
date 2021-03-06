//
// Created by boat on 30-08-20.
//

#include "Ipv4Address.hpp"
#include "../utils/utils.hpp"
#include <iostream>

auto net::Ipv4Address::get_ip_bytes() const -> uint32_t
{
    return ip;
}

net::Ipv4Address::Ipv4Address(uint32_t ip)
    : ip(ip)
{
}

net::Ipv4Address::Ipv4Address(string_view str)
{
    uint32_t x = 0;

    uint8_t s = 0;
    uint8_t c = 0;
    bool ns = false;
    for (auto chr : str) {
        if (http::parser_utils::is_digit(chr)) {
            ns = true;
            uint8_t n = chr - '0';
            c = c * 10 + n;
            if (c < n) {
                throw std::runtime_error("number is greater than 255");
            }
        } else if (chr == '.' && ns) {
            x <<= 8u;
            x |= c;
            c = 0;
            s += 1;
            ns = false;
        } else {
            throw std::runtime_error("invalid character");
        }
    }
    if (ns) {
        for (; s < 4; s += 1) {
            x <<= 8u;
        }
        x |= c;
    }
    ip = utils::bswap32(x);
}

auto net::Ipv4Address::operator==(const net::Ipv4Address& rhs) const -> bool
{
    return ip == rhs.ip;
}

auto net::Ipv4Address::operator!=(const net::Ipv4Address& rhs) const -> bool
{
    return ip != rhs.ip;
}

auto net::Ipv4Address::operator>(const net::Ipv4Address& rhs) const -> bool
{
    return ip > rhs.ip;
}

auto net::Ipv4Address::operator<(const net::Ipv4Address& rhs) const -> bool
{
    return ip < rhs.ip;
}

auto operator<<(utils::StringStream& os, const net::Ipv4Address& sa) -> utils::StringStream&
{
    auto ip = sa.get_ip_bytes();
    auto* x = reinterpret_cast<uint8_t*>(&ip);
    os << std::to_string(x[0])
       << "."
       << std::to_string(x[1])
       << "."
       << std::to_string(x[2])
       << "."
       << std::to_string(x[3]);
    return os;
}
