//
// Created by boat on 31-08-20.
//

#include "FdStringReadFuture.hpp"

ioruntime::FdStringReadFuture::FdStringReadFuture(FileDescriptor&& fd, std::string& str)
    : fd(std::move(fd))
    , str(str)
{
}

auto ioruntime::FdStringReadFuture::poll(futures::Waker&& waker) -> futures::PollResult<void>
{
    char buf[128];

    auto res = fd.poll_read(buf, sizeof(buf), std::move(waker));
    if (res.is_ready()) {
        if (res.get() < 0) {
            throw std::runtime_error("read failed");
        }

        if (res.get() == 0)
            return PollResult<void>::ready();

        str += std::string_view(buf, res.get());
    }
    return PollResult<void>::pending();
}
