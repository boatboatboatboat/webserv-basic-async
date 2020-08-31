//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_SOCKET_HPP
#define WEBSERV_IORUNTIME_SOCKET_HPP

#include "../ioruntime/FileDescriptor.hpp"

namespace net {

class Socket : public ioruntime::FileDescriptor {
public:
    explicit Socket(int fd);
    static Socket uninitialized();
    Socket(Socket&& other) noexcept;
    ~Socket() override;// = default;
    ssize_t read(char* buffer, size_t size) override;
    ssize_t write(const char* buffer, size_t size) override;

private:
    Socket();
};
}

#endif //WEBSERV_IORUNTIME_SOCKET_HPP
