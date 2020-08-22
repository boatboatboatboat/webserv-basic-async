//
// Created by Djevayo Pattij on 8/22/20.
//

#ifndef WEBSERV_IORUNTIME_SOCKETADDR_HPP
#define WEBSERV_IORUNTIME_SOCKETADDR_HPP

// TODO: there should be a net namespace

#include <netinet/in.h>
#include <string>

namespace ioruntime {

class SocketAddr {
public:
    explicit SocketAddr(sockaddr_in addr);
    [[nodiscard]] uint32_t get_ip() const;
    [[nodiscard]] in_port_t get_real_port() const;
    [[nodiscard]] uint16_t get_port() const;
private:
    sockaddr_in addr;
};

}

std::ostream& operator<<(std::ostream& os, const ioruntime::SocketAddr& sa);


#endif //WEBSERV_IORUNTIME_SOCKETADDR_HPP
