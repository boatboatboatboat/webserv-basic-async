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
    ssize_t res;
#ifdef __linux__
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    res = recv(descriptor, buffer, size, MSG_NOSIGNAL);
#endif
#ifdef __APPLE__
    res = recv(descriptor, buffer, size, 0);
#endif
#ifdef DEBUG_SOCKET_INCOMING_PRINTER
    if (res > 0) {
        DBGPRINT("SOCKET " << res << " " << std::string_view((const char*)buffer, res));
    } else if (res == 0) {
        DBGPRINT("SOCKET EOF");
    } else {
        DBGPRINT("SOCKET ERR");
    }
#endif
    return res;
}

auto Socket::write(void const* buffer, size_t size) -> ssize_t
{
    ssize_t res;
#ifdef __linux__
    // MSG_NOSIGNAL disables SIGPIPE being called on a broken pipe
    res = send(descriptor, buffer, size, MSG_NOSIGNAL);
#endif
#ifdef __APPLE__
    res = send(descriptor, buffer, size, 0);
#endif
#ifdef DEBUG_SOCKET_OUTGOING_PRINTER
    if (res > 0) {
        DBGPRINT("SOCKET[" << descriptor << "] " << res << "/" << size << "\n"
                           << std::string_view((const char*)buffer, res));
    } else if (res < 0) {
        DBGPRINT("SOCKET[" << descriptor << "] ERR " << strerror(errno));
    }
#endif
    return res;
}

Socket::Socket(int fd, SocketAddr addr)
    : FileDescriptor(fd)
    , _addr(addr)
{
    TRACEPRINT("socket register " << fd);
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
        TRACEPRINT("socket unregister " << descriptor);
        GlobalIoEventHandler::unregister_special_callbacks(descriptor);
    }
}

auto Socket::get_addr() const -> SocketAddr const&
{
    return _addr.value();
}

}