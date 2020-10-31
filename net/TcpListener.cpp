//
// Created by Djevayo Pattij on 8/3/20.
//

#include "TcpListener.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include "IpAddress.hpp"
#include "SocketAddr.hpp"
#include <netinet/in.h>

// fix funny os x "haha lets error everything" bug
#ifndef SO_NOSIGPIPE
#define SO_NOSIGPIPE 0
#endif

using ioruntime::GlobalIoEventHandler;

namespace net {

TcpListener::TcpListener(IpAddress ip, in_port_t port)
    : addr(sockaddr_in {})
{
    int protocol_specifier;
    int sockaddr_size;

    if (ip.is_v4()) {
        protocol_specifier = AF_INET;
        sockaddr_size = sizeof(sockaddr_in);
    } else if (ip.is_v6()) {
        protocol_specifier = AF_INET6;
        sockaddr_size = sizeof(sockaddr_in6);
    } else {
        throw std::logic_error("unreachable");
    }

    if ((descriptor = socket(protocol_specifier, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error(strerror(errno));

    int enable = 1;
    // SO_REUSEADDR makes used ports (address) of server socket still available when program is killed by signal
    if (setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw std::runtime_error(strerror(errno));

    if (ip.is_v4()) {
        sockaddr_in new_address {};
        // AF_INET is a IPv4 protocol specifier
        new_address.sin_family = protocol_specifier;
        // NOTE: might need to change to big-endian
        new_address.sin_addr.s_addr = ip.get_v4().get_ip_bytes();
        // PORT is specified by server configuration
        new_address.sin_port = htons(port);
        utils::bzero(new_address.sin_zero);
        addr = net::SocketAddr(new_address);
    } else if (ip.is_v6()) {
        sockaddr_in6 new_address {};
        // Protocol specifier (IPv6)
        new_address.sin6_family = protocol_specifier;
        new_address.sin6_port = htons(port);
        new_address.sin6_addr = ip.get_v6().get_ip_posix();
#ifdef __APPLE__
        new_address.sin6_len = sizeof(new_address);
#endif
        new_address.sin6_flowinfo = 0;
        addr = net::SocketAddr(new_address);
    }

    // bind descriptor to address
    if (bind(descriptor, addr.get_addr(), sockaddr_size) < 0) {
        throw std::runtime_error(strerror(errno));
    }
    // start listening
    if (listen(descriptor, SOMAXCONN) < 0) {
        throw std::runtime_error(strerror(errno));
    }

    // register reader callback so the listener can get polled
    BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
    GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
}

auto TcpListener::poll_next(Waker&& waker) -> StreamPollResult<TcpStream>
{
    auto is_ready = connection_ready->lock();

    if (*is_ready) {
        int client;
        sockaddr client_address {};
        auto* client_address_in = reinterpret_cast<sockaddr_in*>(&client_address);
        unsigned int client_address_length = sizeof(client_address);

        if ((client = accept(descriptor, &client_address, &client_address_length)) < 0) {
            WARNPRINT("Unable to accept client");
            BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
            GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
            *is_ready = false;
            throw std::runtime_error(strerror(errno));
        }
        int enable = 1;
        if (setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &enable, sizeof(enable)) < 0) {
            WARNPRINT("noo client doesn't work noo");
        }
        TRACEPRINT("Accepted TCP connection from " << client);

        *is_ready = false;
        BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 1);
        auto stream = TcpStream(client, net::SocketAddr(*client_address_in));
        return StreamPollResult<TcpStream>::ready(std::move(stream));
    } else {
        BoxFunctor cb = BoxFunctor(new SetReadyFunctor(RcPtr(connection_ready)));
        GlobalIoEventHandler::register_reader_callback(descriptor, std::move(cb), true);
        GlobalIoEventHandler::register_reader_callback(descriptor, waker.boxed(), true, 1);
        return StreamPollResult<TcpStream>::pending();
    }
}

auto TcpListener::get_addr() const -> SocketAddr const&
{
    return addr;
}

}