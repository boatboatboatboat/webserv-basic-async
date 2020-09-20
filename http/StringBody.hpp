//
// Created by boat on 8/22/20.
//

#ifndef WEBSERV_STRINGBODY_HPP
#define WEBSERV_STRINGBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"

namespace http {

class StringBody : public ioruntime::IAsyncRead {
public:
    explicit StringBody(std::string body);
    auto poll_read(char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> override;

private:
    std::string body;
    size_t written {};
};

}
#endif //WEBSERV_STRINGBODY_HPP
