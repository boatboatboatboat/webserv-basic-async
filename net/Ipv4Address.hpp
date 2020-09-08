//
// Created by boat on 30-08-20.
//

#ifndef WEBSERV_IPV4ADDRESS_HPP
#define WEBSERV_IPV4ADDRESS_HPP

#include <cstdint>
#include <string>

using std::string_view;

namespace net {

class Ipv4Address {
public:
    Ipv4Address() = delete;
    explicit Ipv4Address(string_view str);
    explicit Ipv4Address(uint32_t ip);
    [[nodiscard]] auto get_ip_bytes() const -> uint32_t;

private:
    uint32_t ip;
};

}

auto operator<<(std::ostream& os, const net::Ipv4Address& sa) -> std::ostream&;

#endif //WEBSERV_IPV4ADDRESS_HPP
