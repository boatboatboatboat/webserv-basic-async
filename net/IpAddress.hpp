//
// Created by Djevayo Pattij on 9/6/20.
//

#ifndef WEBSERV_NET_IPADDRESS_HPP
#define WEBSERV_NET_IPADDRESS_HPP

#include "Ipv4Address.hpp"
#include "Ipv6Address.hpp"
#include <cstdint>

namespace net {
class IpAddress {
public:
    static IpAddress v4(uint32_t ip);
    static IpAddress v4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
    static IpAddress v6(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7, uint16_t b8);
    [[nodiscard]] bool is_v4() const;
    [[nodiscard]] bool is_v6() const;
    [[nodiscard]] Ipv4Address get_v4() const;
    [[nodiscard]] Ipv6Address get_v6() const;
private:
    enum Tag {
        Ipv4,
        Ipv6
    } tag;
    union {
        Ipv4Address in_v4;
        Ipv6Address in_v6;
    };
    IpAddress(Tag tag, uint64_t ip, uint64_t ip2);

};
}

#endif //WEBSERV_NET_IPADDRESS_HPP
