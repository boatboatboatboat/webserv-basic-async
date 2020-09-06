//
// Created by Djevayo Pattij on 8/22/20.
//

#ifndef WEBSERV_IORUNTIME_SOCKETADDR_HPP
#define WEBSERV_IORUNTIME_SOCKETADDR_HPP

#include <netinet/in.h>
#include <string>

namespace net {

class SocketAddr {
public:
    explicit SocketAddr(sockaddr_in addr);
    explicit SocketAddr(sockaddr_in6 addr);
    [[nodiscard]] uint32_t get_v4() const;
    [[nodiscard]] in6_addr get_v6() const;
    [[nodiscard]] bool is_v4() const;
    [[nodiscard]] bool is_v6() const;
    [[nodiscard]] in_port_t get_real_port() const;
    [[nodiscard]] uint16_t get_port() const;
    [[nodiscard]] sockaddr* get_addr() const;

private:
    enum IpTag {
        IpV4,
        IpV6
    } tag;
    union {
        sockaddr_in addr_v4;
        sockaddr_in6 addr_v6;
    };
};
}

std::ostream& operator<<(std::ostream& os, const net::SocketAddr& sa);

#endif //WEBSERV_IORUNTIME_SOCKETADDR_HPP
