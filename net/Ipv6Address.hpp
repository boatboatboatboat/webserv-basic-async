//
// Created by Djevayo Pattij on 9/6/20.
//

#ifndef WEBSERV_NET_IPV6ADDRESS_HPP
#define WEBSERV_NET_IPV6ADDRESS_HPP

#include <cstdint>
#include <netinet/in.h>
#include <string>

using std::string_view;

namespace net {
class Ipv6Address {
public:
    Ipv6Address() = delete;
    explicit Ipv6Address(uint64_t b1, uint64_t b2);
    explicit Ipv6Address(string_view str);
    explicit Ipv6Address(in6_addr addr);
    [[nodiscard]] auto get_ip_posix() const -> in6_addr;

    auto operator==(Ipv6Address const& rhs) const -> bool;
    auto operator!=(Ipv6Address const& rhs) const -> bool;
    auto operator>(Ipv6Address const& rhs) const -> bool;
    auto operator<(Ipv6Address const& rhs) const -> bool;

private:
    in6_addr ip {};
};
}

auto operator<<(std::ostream& os, const net::Ipv6Address& sa) -> std::ostream&;

#endif //WEBSERV_NET_IPV6ADDRESS_HPP
