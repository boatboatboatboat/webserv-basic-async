//
// Created by Djevayo Pattij on 8/21/20.
//

#include "Socket.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include <sys/socket.h>

#include <fcntl.h>

class FunnyFunctor : public Functor {
public:
    void operator()() override
    {
        INFOPRINT("Funny call");
    }
};

using ioruntime::GlobalIoEventHandler;

namespace net {

auto Socket::read(void* buffer, size_t size) -> ssize_t
{
    // shhh don't tell anyone
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0)
        return -1;
#ifdef __linux__
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    return recv(descriptor, buffer, size, MSG_NOSIGNAL);
#endif
#ifdef __APPLE__
    return recv(descriptor, buffer, size, 0);
#endif
}

auto Socket::write(void const* buffer, size_t size) -> ssize_t
{
    // shhh don't tell anyone
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0)
        return -1;
#ifdef __linux__
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    return send(descriptor, buffer, size, MSG_NOSIGNAL);
#endif
#ifdef __APPLE__
    return send(descriptor, buffer, size, 0);
#endif
}

Socket::Socket(int fd, SocketAddr addr)
    : FileDescriptor(fd)
    , _addr(addr)
{
    GlobalIoEventHandler::register_special_callback(descriptor, BoxFunctor(new FunnyFunctor()), false, 0);
}

Socket::Socket(Socket&& other) noexcept
    : FileDescriptor(std::move(other))
{
    if (descriptor >= 0) {
        GlobalIoEventHandler::unregister_special_callbacks(descriptor);
        GlobalIoEventHandler::register_special_callback(descriptor, BoxFunctor(new FunnyFunctor()), false, 0);
    }
}

Socket::~Socket()
{
    if (descriptor >= 0) {
        TRACEPRINT("unregister " << descriptor);
        GlobalIoEventHandler::unregister_special_callbacks(descriptor);
    }
}

auto Socket::get_addr() const -> SocketAddr const&
{
    return _addr.value();
}

}