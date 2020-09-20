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
    explicit ForEachFuture(St&& stream, void (*function)(Fp&), void (*_eh)(std::exception& e));
    ForEachFuture(ForEachFuture&& other) noexcept ;
    ~ForEachFuture() override = default;
    auto poll(Waker&& waker) -> PollResult<void> override;

private:
    St _stream;
    void (*_function)(Fp&);
    void (*_eh)(std::exception& e);
};

template <typename St, typename Fp>
ForEachFuture<St, Fp>::ForEachFuture(St&& stream, void (*function)(Fp&))
    : _stream(std::move(stream))
    , _function(function)
    , _eh(nullptr)
{
}

template <typename St, typename Fp>
auto
ForEachFuture<St, Fp>::poll(Waker&& waker) -> PollResult<void>
{
    try {
        auto result = _stream.poll_next(std::move(waker));
        switch (result.get_status()) {
        case StreamPollResult<Fp>::Pending: {
            return PollResult<void>::pending();
        } break;
        case StreamPollResult<Fp>::Ready: {
            _function(result.get());
            return PollResult<void>::pending();
        } break;
        case StreamPollResult<Fp>::Finished: {
            return PollResult<void>::ready();
        } break;
        }
        return PollResult<void>::pending();
    } catch (std::exception& e) {
        if (_eh != nullptr) {
            _eh(e);
            return PollResult<void>::pending();
        } else {
            throw;
        }
    }
}

template <typename St, typename Fp>
ForEachFuture<St, Fp>::ForEachFuture(ForEachFuture&& other)noexcept
    : _stream(std::move(other._stream))
    , _function(other._function)
    , _eh(other._eh)
{
}

template <typename St, typename Fp>
ForEachFuture<St, Fp>::ForEachFuture(St&& stream, void (*function)(Fp&), void (*_eh)(std::exception&)): _stream(std::move(stream))
    , _function(function)
    , _eh(_eh)
{
}

}

#endif //WEBSERV_FUTURES_FOREACHFUTURE_HPP
