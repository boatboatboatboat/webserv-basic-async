//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_IORUNTIME_TCPSTREAM_HPP
#define WEBSERV_IORUNTIME_TCPSTREAM_HPP

#include "../futures/PollResult.hpp"
#include "Socket.hpp"
#include "SocketAddr.hpp"
#include <sys/socket.h>

using futures::IFuture;
using futures::PollResult;

namespace ioruntime {

class TcpStream;

class TcpStreamResponseFuture : public IFuture<void> {
public:
    TcpStreamResponseFuture(TcpStream&& stream, std::string (*responder)(std::string& str));
    PollResult<void> poll(Waker&& waker) override;

private:
    enum State {
        Reading,
        Writing
    };
    static const int BUFFER_SIZE = 64;
    size_t written = 0;
    State state = Reading;
    char buffer[BUFFER_SIZE] {};
    std::string message;
    std::string (*responder)(std::string& str);
    Socket socket;
};

class TcpStream {
    friend class TcpStreamResponseFuture;

public:
    explicit TcpStream(int fd, SocketAddr address);
    TcpStreamResponseFuture respond(std::string (*fp)(std::string& str)) &&;

    [[nodiscard]] SocketAddr const& get_addr() const;
    static TcpStream uninitialized();

private:
    TcpStream();
    Socket socket;
    SocketAddr address;
};

}
#endif //WEBSERV_IORUNTIME_TCPSTREAM_HPP
