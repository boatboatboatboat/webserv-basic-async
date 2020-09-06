//
// Created by Djevayo Pattij on 9/6/20.
//

#include "IpAddress.hpp"

namespace net {

IpAddress::IpAddress(Tag tag, uint64_t ip, uint64_t ip2):
    tag(tag)
{
    if (tag == Ipv4) {
        in_v4 = Ipv4Address(ip);
    } else if (tag == Ipv6) {
        in_v6 = Ipv6Address(ip, ip2);
    }
}

IpAddress IpAddress::v4(uint32_t ip)
{
    return IpAddress(IpAddress::Ipv4, ip, 0);
}

IpAddress IpAddress::v6(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7, uint16_t b8)
{
    uint64_t ip = b1;
    ip <<= 8u;
    ip |= b2;
    ip <<= 8u;
    ip |= b3;
    ip <<= 8u;
    ip |= b4;
    uint64_t ip2 = b5;
    ip2 <<= 8u;
    ip2 |= b6;
    ip2 <<= 8u;
    ip2 |= b7;
    ip2 <<= 8u;
    ip2 |= b8;
    return IpAddress(IpAddress::Ipv6, ip, ip2);
}

bool IpAddress::is_v4() const
{
    return tag == Ipv4;
}

bool IpAddress::is_v6() const
{
    return tag == Ipv6;
}

Ipv4Address IpAddress::get_v4() const
{
    return in_v4;
}

Ipv6Address IpAddress::get_v6() const
{
    return in_v6;
}

IpAddress IpAddress::v4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    uint32_t ip = b1;
    ip <<= 8u;
    ip |= b2;
    ip <<= 8u;
    ip |= b3;
    ip <<= 8u;
    ip |= b4;

    return IpAddress(IpAddress::Ipv4, ip, 0);
}

}
