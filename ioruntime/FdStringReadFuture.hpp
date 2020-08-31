//
// Created by boat on 31-08-20.
//

#ifndef WEBSERV_IORUNTIME_FDSTRINGREADFUTURE_HPP
#define WEBSERV_IORUNTIME_FDSTRINGREADFUTURE_HPP

#include "../futures/IFuture.hpp"
#include "FileDescriptor.hpp"
#include <string>

using futures::IFuture;

namespace ioruntime {

class FdStringReadFuture : public IFuture<void> {
public:
    explicit FdStringReadFuture(FileDescriptor&& fd, std::string& str);
    futures::PollResult<void> poll(futures::Waker&& waker) override;
private:
    FileDescriptor fd;
    std::string& str;
};

}

#endif //WEBSERV_IORUNTIME_FDSTRINGREADFUTURE_HPP
