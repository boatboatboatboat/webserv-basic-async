//
// Created by Djevayo Pattij on 8/22/20.
//

#ifndef WEBSERV_IORUNTIME_SOCKETADDR_HPP
#define WEBSERV_IORUNTIME_SOCKETADDR_HPP

#include "../utils/StringStream.hpp"
#include <netinet/in.h>
#include <string>

namespace net {

class SocketAddr {
public:
    explicit SocketAddr(sockaddr addr);
    explicit SocketAddr(sockaddr_in addr);
    explicit SocketAddr(sockaddr_in6 addr);
    [[nodiscard]] auto get_v4() const -> uint32_t;
    [[nodiscard]] auto get_v6() const -> in6_addr;
    [[nodiscard]] auto is_v4() const -> bool;
    [[nodiscard]] auto is_v6() const -> bool;
    [[nodiscard]] auto get_real_port() const -> in_port_t;
    [[nodiscard]] auto get_port() const -> uint16_t;
    [[nodiscard]] auto get_addr() const -> sockaddr*;

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

auto operator<<(utils::StringStream& os, const net::SocketAddr& sa) -> utils::StringStream&;

#endif //WEBSERV_IORUNTIME_SOCKETADDR_HPP
