//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_FOREACHFUTURE_HPP
#define WEBSERV_FUTURES_FOREACHFUTURE_HPP

#include "futures.hpp"

namespace {
    using std::move;
}

/*
 * ForEachFutures are executed purely for their side effects -
 * they do not forward stream results!
 */

namespace futures {
template <typename St>
class ForEachFuture : public IFuture<void> {
public:
    typedef std::function<void(typename St::result_type&&)> callback_type;
    typedef typename St::result_type argument_type;
    explicit ForEachFuture(St&& stream, callback_type function);
    explicit ForEachFuture(St&& stream, callback_type function, void (*_eh)(std::exception& e));
    ForEachFuture(ForEachFuture&& other) noexcept;
    ~ForEachFuture() override = default;
    auto poll(Waker&& waker) -> PollResult<void> override;

private:
    St _stream;
    callback_type _function;
    void (*_eh)(std::exception& e);
};

template <typename St>
ForEachFuture<St>::ForEachFuture(St&& stream, callback_type function)
    : _stream(std::move(stream))
    , _function(function)
    , _eh(nullptr)
{
}

template <typename St>
auto ForEachFuture<St>::poll(Waker&& waker) -> PollResult<void>
{
    try {
        auto result = _stream.poll_next(std::move(waker));
        switch (result.get_status()) {
            case StreamPollResult<argument_type>::Pending: {
                return PollResult<void>::pending();
            } break;
            case StreamPollResult<argument_type>::Ready: {
                _function(move(result.get()));
                return PollResult<void>::pending();
            } break;
            case StreamPollResult<argument_type>::Finished: {
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

template <typename St>
ForEachFuture<St>::ForEachFuture(ForEachFuture&& other) noexcept
    : _stream(move(other._stream))
    , _function(other._function)
    , _eh(other._eh)
{
}

template <typename St>
ForEachFuture<St>::ForEachFuture(St&& stream, callback_type function, void (*_eh)(std::exception&))
    : _stream(move(stream))
    , _function(function)
    , _eh(_eh)
{
}

}

#endif //WEBSERV_FUTURES_FOREACHFUTURE_HPP
