//
// Created by boat on 30-08-20.
//

#ifndef WEBSERV_IPV4ADDRESS_HPP
#define WEBSERV_IPV4ADDRESS_HPP

#include <cstdint>
namespace net {

class Ipv4Address {
public:
    Ipv4Address() = delete;
    explicit Ipv4Address(uint32_t ip);
    [[nodiscard]] uint32_t get_ip_bytes() const;
private:
    uint32_t ip;
};

}

#endif //WEBSERV_IPV4ADDRESS_HPP
