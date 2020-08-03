//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_FOREACHFUTURE_HPP
#define WEBSERV_FUTURES_FOREACHFUTURE_HPP

#include "futures.hpp"

/*
 * ForEachFutures are executed purely for their side effects -
 * they do not forward stream results!
 */

namespace futures {
template <typename St, typename Fp>
class ForEachFuture : public IFuture<void> {
public:
    explicit ForEachFuture(St&& stream, void (*function)(Fp&));
    PollResult<void> poll(Waker&& waker) override;

private:
    St _stream;
    void (*_function)(Fp&);
};

template <typename St, typename Fp>
ForEachFuture<St, Fp>::ForEachFuture(St&& stream, void (*function)(Fp&))
{
    _stream = std::move(stream);
    _function = function;
}

template <typename St, typename Fp>
PollResult<void>
ForEachFuture<St, Fp>::poll(Waker&& waker)
{
    StreamPollResult<Fp> result = _stream.poll_next(std::move(waker));

    switch (result.get_status()) {
    case StreamPollResult<Fp>::Pending: {
        return PollResult<void>::pending();
    } break;
    case StreamPollResult<Fp>::Ready: {
        _function(result.get());
        return PollResult<void>::pending();
    } break;
    case StreamPollResult<Fp>::Completed: {
        return PollResult<void>::ready();
    } break;
    }
    return PollResult<void>::pending();
}
}

#endif //WEBSERV_FUTURES_FOREACHFUTURE_HPP
