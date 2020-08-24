//
// Created by Djevayo Pattij on 8/21/20.
//

#include "Socket.hpp"
#include "GlobalIoEventHandler.hpp"
#include <sys/socket.h>


#include <fcntl.h>

namespace ioruntime {

ssize_t Socket::read(char* buffer, size_t size)
{
    // shhh don't tell anyone
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0)
        return -1;
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    return recv(descriptor, buffer, size, MSG_NOSIGNAL);
}
ssize_t Socket::write(const char* buffer, size_t size)
{
    // shhh don't tell anyone
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0)
        return -1;
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    return send(descriptor, buffer, size, MSG_NOSIGNAL);
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
Socket::~Socket()
{
}

}