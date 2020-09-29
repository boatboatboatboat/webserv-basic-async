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
    explicit Socket(int fd, SocketAddr addr);
    static auto uninitialized() -> Socket;
    Socket(Socket&& other) noexcept;
    ~Socket() override; // = default;
    auto read(char* buffer, size_t size) -> ssize_t override;
    auto write(const char* buffer, size_t size) -> ssize_t override;
    [[nodiscard]] auto get_addr() const -> SocketAddr const&;

private:
    Socket();
    optional<SocketAddr> _addr;
};

}

#endif //WEBSERV_IORUNTIME_SOCKET_HPP
