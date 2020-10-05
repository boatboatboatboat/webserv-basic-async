//
// Created by Djevayo Pattij on 8/26/20.
//

#include "TimeoutFuture.hpp"
#include "../func/SetReadyFunctor.hpp"
#include "../futures/PollResult.hpp"
#include "GlobalTimeoutEventHandler.hpp"

futures::PollResult<void> ioruntime::TimeoutFuture::poll(futures::Waker&& waker)
{
    if (dead)
        return futures::PollResult<void>::ready();
    if (!waker_set) {
        waker_set = true;
        ioruntime::GlobalTimeoutEventHandler::register_timeout_real(tick, waker.boxed());
    }
    auto finished = timed_out->lock();
    if (*finished) {
        DBGPRINT("TF complete");
        return futures::PollResult<void>::ready();
    } else {
        DBGPRINT("TF pend");
        return futures::PollResult<void>::pending();
    }
}

ioruntime::TimeoutFuture::TimeoutFuture(uint64_t ms)
{
    tick = TimeoutEventHandler::get_time_ms() + ms;
    ioruntime::GlobalTimeoutEventHandler::register_timeout_real(tick, BoxFunctor(new SetReadyFunctor(RcPtr(timed_out))));
}

ioruntime::TimeoutFuture ioruntime::TimeoutFuture::dead_timer()
{
    return TimeoutFuture();
}

ioruntime::TimeoutFuture::TimeoutFuture()
    : dead(true)
    , timed_out(RcPtr<Mutex<bool>>::uninitialized())
    , tick(0)
{
}
