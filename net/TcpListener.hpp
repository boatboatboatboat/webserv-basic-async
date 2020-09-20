//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_IORUNTIME_TCPLISTENER_HPP
#define WEBSERV_IORUNTIME_TCPLISTENER_HPP

#include "../func/SetReadyFunctor.hpp"
#include "../futures/IStreamExt.hpp"
#include "IpAddress.hpp"
#include "SocketAddr.hpp"
#include "TcpStream.hpp"

using futures::IStreamExt;
using futures::StreamPollResult;

namespace net {

class TcpListener : public futures::IStreamExt<TcpStream> {
public:
    explicit TcpListener(IpAddress ip, uint16_t port);
    [[nodiscard]] auto get_addr() const -> SocketAddr const&;
    auto poll_next(Waker&& waker) -> futures::StreamPollResult<TcpStream> override;

private:
    int descriptor;
    SocketAddr addr;
    boxed::RcPtr<Mutex<bool>> connection_ready;
};

}

#endif //WEBSERV_IORUNTIME_TCPLISTENER_HPP
