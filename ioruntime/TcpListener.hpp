//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_IORUNTIME_TCPLISTENER_HPP
#define WEBSERV_IORUNTIME_TCPLISTENER_HPP

#include "../futures/IStreamExt.hpp"
#include "TcpStream.hpp"
#include "../func/SetReadyFunctor.hpp"

using futures::IStreamExt;
using futures::StreamPollResult;

namespace ioruntime {
class TcpListener : public IStreamExt<TcpStream> {
public:
    explicit TcpListener(uint16_t port);
    [[nodiscard]] SocketAddr const& get_addr() const;
    StreamPollResult<TcpStream> poll_next(Waker&& waker) override;

private:
    int descriptor;
    SocketAddr addr;
    RcPtr<Mutex<bool>> connection_ready;
};

}

#endif //WEBSERV_IORUNTIME_TCPLISTENER_HPP
