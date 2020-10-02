//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_SOCKET_HPP
#define WEBSERV_IORUNTIME_SOCKET_HPP

#include "../ioruntime/FileDescriptor.hpp"
#include "SocketAddr.hpp"

using option::optional;

namespace net {

class Socket : public ioruntime::FileDescriptor {
public:
    Socket() = delete;
    explicit Socket(int fd, SocketAddr addr);
    Socket(Socket&& other) noexcept;
    ~Socket() override; // = default;
    auto read(void* buffer, size_t size) -> ssize_t override;
    auto write(void const* buffer, size_t size) -> ssize_t override;
    [[nodiscard]] auto get_addr() const -> SocketAddr const&;

private:
    optional<SocketAddr> _addr;
};

}

#endif //WEBSERV_IORUNTIME_SOCKET_HPP
