//
// Created by boat on 09-10-20.
//

#ifndef WEBSERV_HTTP_FUTURESEREADER_HPP
#define WEBSERV_HTTP_FUTURESEREADER_HPP

#include "../futures/IFuture.hpp"
#include "../ioruntime/IAsyncRead.hpp"

namespace http {

namespace {
    using futures::IFuture;
    using ioruntime::IoResult;
}

template <typename Fut>
class FutureSeReader : public ioruntime::IAsyncRead {
    typedef typename std::enable_if<std::is_base_of<IFuture<void>, Fut>::value>::type future_type;

public:
    explicit FutureSeReader(Fut&& fut);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    Fut _fut;
};

template <typename Fut>
FutureSeReader<Fut>::FutureSeReader(Fut&& fut)
    : _fut(move(fut))
{
}

template <typename Fut>
auto FutureSeReader<Fut>::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    (void)buffer;
    auto poll_result = _fut.poll(std::move(waker));
    if (poll_result.is_ready()) {
        return PollResult<IoResult>::ready(IoResult::eof());
    } else {
        return PollResult<IoResult>::pending();
    }
}

}

#endif //WEBSERV_HTTP_FUTURESEREADER_HPP
