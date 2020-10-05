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
    uint8_t buf[128];

    auto res = fd.poll_read(span(buf, sizeof(buf)), std::move(waker));
    if (res.is_ready()) {
        if (res.get().is_error()) {
            throw std::runtime_error("read failed");
        }

        if (res.get().is_eof())
            return PollResult<void>::ready();

        str += std::string_view((char *)buf, res.get().get_bytes_read());
    }
    return PollResult<void>::pending();
}
