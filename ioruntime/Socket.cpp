//
// Created by Djevayo Pattij on 8/21/20.
//

#include "Socket.hpp"
#include "GlobalIoEventHandler.hpp"
#include <sys/socket.h>

namespace ioruntime {

ssize_t Socket::read(char* buffer, size_t size)
{
    return recv(descriptor, buffer, size, 0);
}
ssize_t Socket::write(const char* buffer, size_t size)
{
    return send(descriptor, buffer, size, 0);
}

Socket::Socket(int fd)
    : FileDescriptor(fd)
{
}

Socket::Socket(Socket&& other) noexcept
    : FileDescriptor(std::move(other))
{
}

Socket::Socket()
    : FileDescriptor(FileDescriptor::uninitialized())
{
}

Socket Socket::uninitialized()
{
    return Socket();
}

}