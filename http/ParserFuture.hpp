//
// Created by boat on 8/22/20.
//

#ifndef WEBSERV_PARSERFUTURE_HPP
#define WEBSERV_PARSERFUTURE_HPP

#include "../futures/IFuture.hpp"
#include "../net/TcpStream.hpp"
#include "HttpRequest.hpp"

using futures::IFuture;
using futures::PollResult;
using net::TcpStream;

namespace http {
// forward declarations
class HttpRequest;

// classes
class ParserFuture : public IFuture<HttpRequest> {
public:
    ParserFuture() = delete;
    explicit ParserFuture(net::TcpStream* stream, size_t limit = 8192);
    PollResult<HttpRequest> poll(Waker&& waker) override;

    net::TcpStream* stream;
private:
    const size_t limit;
    std::string buffer;
};

}

#endif //WEBSERV_PARSERFUTURE_HPP
