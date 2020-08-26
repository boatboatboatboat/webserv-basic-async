//
// Created by Djevayo Pattij on 8/3/20.
//

#include "TcpListener.hpp"
#include "GlobalIoEventHandler.hpp"
#include <netinet/in.h>

namespace ioruntime {

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
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
}

StreamPollResult<TcpStream> TcpListener::poll_next(Waker&& waker)
{
    // INFOPRINT("pollnext Listener");
    // register immediately;
    auto is_ready = connection_ready->lock();

    if (*is_ready) {
        int client;
        sockaddr client_address {};
        auto* client_address_in = reinterpret_cast<sockaddr_in*>(&client_address);
        unsigned int client_address_length = sizeof(client_address);

        if ((client = accept(descriptor, &client_address, &client_address_length)) < 0) {
            WARNPRINT("Denied client");
            BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
            GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
            throw std::runtime_error(strerror(errno));
        }
        DBGPRINT("accepted client " << client);

        *is_ready = false;
        BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 1);
//        for (;;) {
//            try {
//                INFOPRINT("stream test");
                auto stream = TcpStream(client, SocketAddr(*client_address_in));
                return StreamPollResult<TcpStream>::ready(std::move(stream));
//            } catch (std::exception& e) {
//                // FIXME: shit code
//                             INFOPRINT("Retry for " << client);
//                if (strstr(e.what(), "FD_SETSIZE") != nullptr) {
//                    int newfd = dup(client);
//                    close(client);
//                    client = newfd;
//                    continue;
//                } else {
//                    throw;
//                }
//            }
//        }
    } else {
        BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 1);
        return StreamPollResult<TcpStream>::pending(TcpStream::uninitialized());
    }
}

SocketAddr const& TcpListener::get_addr() const
{
    return addr;
}

}