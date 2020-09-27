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
    static auto v4(uint32_t ip) -> IpAddress;
    static auto v4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) -> IpAddress;
    static auto v6(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7, uint16_t b8) -> IpAddress;
    static auto from_str(std::string_view str) -> IpAddress;
    [[nodiscard]] auto is_v4() const -> bool;
    [[nodiscard]] auto is_v6() const -> bool;
    [[nodiscard]] auto get_v4() const -> Ipv4Address;
    [[nodiscard]] auto get_v6() const -> Ipv6Address;

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
    explicit IpAddress(Ipv4Address ip);
    explicit IpAddress(Ipv6Address ip);
};
}

auto operator<<(std::ostream& os, const net::IpAddress& sa) -> std::ostream&;

#endif //WEBSERV_NET_IPADDRESS_HPP
