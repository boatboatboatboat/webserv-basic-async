//
// Created by Djevayo Pattij on 9/6/20.
//

#ifndef WEBSERV_NET_IPV6ADDRESS_HPP
#define WEBSERV_NET_IPV6ADDRESS_HPP

#include <cstdint>
#include <netinet/in.h>

namespace net {
class Ipv6Address {
public:
    Ipv6Address() = delete;
    explicit Ipv6Address(uint64_t b1, uint64_t b2);
    [[nodiscard]] in6_addr get_ip_posix() const;
private:
    in6_addr ip;
};
}

#endif //WEBSERV_NET_IPV6ADDRESS_HPP
