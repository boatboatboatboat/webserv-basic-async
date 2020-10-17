//
// Created by Djevayo Pattij on 8/26/20.
//

#include "TimeoutFuture.hpp"
#include "../func/SetReadyFunctor.hpp"
#include "../futures/PollResult.hpp"
#include "GlobalTimeoutEventHandler.hpp"

class CombinedFunctor : public Functor {
public:
    CombinedFunctor(Waker&& waker, SetReadyFunctor&& srf)
        : _waker(waker)
        , _srf(srf)
    {
    }
    void operator()() override
    {
        _srf();
        _waker();
    }

private:
    Waker _waker;
    SetReadyFunctor _srf;
};

futures::PollResult<void> ioruntime::TimeoutFuture::poll(futures::Waker&& waker)
{
    if (_dead)
        return futures::PollResult<void>::ready();
    _connection = ioruntime::GlobalTimeoutEventHandler::register_timeout_real(
        _tick,
        BoxFunctor(new CombinedFunctor(std::move(waker), SetReadyFunctor(RcPtr(_timed_out)))));
    auto finished = _timed_out->lock();
    if (*finished) {
        return futures::PollResult<void>::ready();
    } else {
        return futures::PollResult<void>::pending();
    }
}

ioruntime::TimeoutFuture::TimeoutFuture(uint64_t ms)
{
    _tick = TimeoutEventHandler::get_time_ms() + ms;
    _connection = ioruntime::GlobalTimeoutEventHandler::register_timeout_real(
        _tick,
        BoxFunctor(new SetReadyFunctor(RcPtr(_timed_out))));
}

ioruntime::TimeoutFuture::~TimeoutFuture()
{
}

ioruntime::TimeoutFuture::TimeoutFuture(ioruntime::TimeoutFuture&& other) noexcept
    : _dead(other._dead)
    , _timed_out(std::move(other._timed_out))
    , _connection(std::move(other._connection))
    , _tick(other._tick)
{
}

ioruntime::TimeoutFuture& ioruntime::TimeoutFuture::operator=(ioruntime::TimeoutFuture&& other) noexcept
{
    if (this == &other) {
        return *this;
    }
    _dead = other._dead;
    _timed_out = std::move(other._timed_out);
    _connection = std::move(other._connection);
    _tick = std::move(other._tick);
    return *this;
}
