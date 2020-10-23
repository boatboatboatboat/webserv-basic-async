//
// Created by Djevayo Pattij on 9/6/20.
//

#include "IpAddress.hpp"
#include <iostream>

namespace net {

IpAddress::IpAddress(Tag tag, uint64_t ip, uint64_t ip2)
    : tag(tag)
{
    if (tag == Ipv4) {
        in_v4 = Ipv4Address(ip);
    } else if (tag == Ipv6) {
        in_v6 = Ipv6Address(ip, ip2);
    }
}

IpAddress::IpAddress(Ipv4Address ip)
    : tag(Ipv4)
{
    in_v4 = ip;
}

IpAddress::IpAddress(Ipv6Address ip)
    : tag(Ipv6)
{
    in_v6 = ip;
}

auto IpAddress::v4(uint32_t ip) -> IpAddress
{
    return IpAddress(IpAddress::Ipv4, ip, 0);
}

auto IpAddress::v6(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7, uint16_t b8) -> IpAddress
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

auto IpAddress::is_v4() const -> bool
{
    return tag == Ipv4;
}

auto IpAddress::is_v6() const -> bool
{
    return tag == Ipv6;
}

auto IpAddress::get_v4() const -> Ipv4Address
{
    return in_v4;
}

auto IpAddress::get_v6() const -> Ipv6Address
{
    return in_v6;
}

auto IpAddress::v4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) -> IpAddress
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

auto IpAddress::from_str(std::string_view str) -> IpAddress
{
    try {
        return IpAddress(Ipv4Address(str));
    } catch (std::exception& e) {
        try {
            return IpAddress(Ipv6Address(str));
        } catch (std::exception& e) {
            throw std::runtime_error("could not convert from string");
        }
    }
}

auto IpAddress::operator==(const IpAddress& rhs) const -> bool
{
    if (tag != rhs.tag) {
        return false;
    }
    switch (tag) {
        case Ipv4: {
            return get_v4() == rhs.get_v4();
        } break;
        case Ipv6: {
            return get_v6() == rhs.get_v6();
        } break;
    }
}

auto IpAddress::operator!=(const IpAddress& rhs) const -> bool
{
    return !(*this == rhs);
}

auto IpAddress::operator>(const IpAddress& rhs) const -> bool
{
    if (tag > rhs.tag) {
        return true;
    }
    switch (tag) {
        case Ipv4: {
            return get_v4() > rhs.get_v4();
        } break;
        case Ipv6: {
            return get_v6() > rhs.get_v6();
        } break;
    }
}

auto IpAddress::operator<(const IpAddress& rhs) const -> bool
{
    if (tag < rhs.tag) {
        return true;
    }
    switch (tag) {
        case Ipv4: {
            return get_v4() < rhs.get_v4();
        } break;
        case Ipv6: {
            return get_v6() < rhs.get_v6();
        } break;
    }
}

}

auto operator<<(utils::StringStream& os, const net::IpAddress& sa) -> utils::StringStream&
{
    if (sa.is_v4()) {
        os << sa.get_v4();
    } else if (sa.is_v6()) {
        os << sa.get_v6();
    }
    return os;
}