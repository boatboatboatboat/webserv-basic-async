//
// Created by Djevayo Pattij on 8/3/20.
//

#include "TcpListener.hpp"
#include "GlobalIoEventHandler.hpp"
#include <netinet/in.h>

namespace ioruntime {

TcpListener::SetReadyFunctor::SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source)
    : ready_mutex(std::move(cr_source))
{
}

void TcpListener::SetReadyFunctor::operator()()
{
    auto cread_guard = ready_mutex->lock();
    *cread_guard = true;
}

TcpListener::TcpListener(in_port_t port)
    : addr({})
{
    sockaddr_in new_address {};

    if ((descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error(strerror(errno));
    int enable = 1;
    // SO_REUSEADDR makes used ports (address) of server socket still available when program is killed by signal
    if (setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw std::runtime_error(strerror(errno));
    // AF_INET is a IPv4 protocol specifier
    new_address.sin_family = AF_INET;
    // no need to convert to big-endian because INADDR_ANY macro expands to 0.0.0.0
    // need to convert to big-endian otherwise
    new_address.sin_addr.s_addr = INADDR_ANY;
    // PORT is specified by server configuration
    new_address.sin_port = htons(port);
    // FIXME: use libft bzero
    bzero(new_address.sin_zero, sizeof(new_address.sin_zero));
    // bind descriptor to address
    if (bind(descriptor, reinterpret_cast<sockaddr*>(&new_address), sizeof(new_address)) < 0)
        throw std::runtime_error(strerror(errno));
    // start listening
    if (listen(descriptor, SOMAXCONN) < 0)
        throw std::runtime_error(strerror(errno));

    addr = SocketAddr(new_address);

    // register reader callback so the listener can get polled
    BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), false);

    // debug: print fd and port
    DBGPRINT("TCP server listening on " << addr << " bound to fd " << descriptor);
}

StreamPollResult<TcpStream> TcpListener::poll_next(Waker&& waker)
{
    // register immediately;
    auto is_ready = connection_ready->lock();

    if (*is_ready) {
        int client;
        sockaddr client_address {};
        auto* client_address_in = reinterpret_cast<sockaddr_in*>(&client_address);
        unsigned int client_address_length = sizeof(client_address);

        if ((client = accept(descriptor, &client_address, &client_address_length)) < 0) {
            throw std::runtime_error(strerror(errno));
        }

        *is_ready = false;
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), false, 1);
        return StreamPollResult<TcpStream>::ready(TcpStream(client, SocketAddr(*client_address_in)));
    } else {
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), false, 1);
        return StreamPollResult<TcpStream>::pending(TcpStream::uninitialized());
    }
}

SocketAddr const& TcpListener::get_addr() const
{
    return addr;
}

}