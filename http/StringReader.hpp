//
// Created by boat on 8/22/20.
//

#ifndef WEBSERV_STRINGBODY_HPP
#define WEBSERV_STRINGBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"

using ioruntime::IoResult;

namespace http {

class StringReader : public ioruntime::IAsyncRead {
public:
    explicit StringReader(std::string body, bool stream_like = false);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    bool stream_like;
    std::string body;
    size_t written {};
};

}
#endif //WEBSERV_STRINGBODY_HPP
