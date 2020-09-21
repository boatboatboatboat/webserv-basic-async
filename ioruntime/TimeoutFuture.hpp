//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP
#define WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP

#include "../boxed/RcPtr.hpp"
#include "../futures/IFuture.hpp"
#include "../mutex/mutex.hpp"

using boxed::RcPtr;
using futures::IFuture;
using mutex::Mutex;

namespace ioruntime {

class TimeoutFuture : public IFuture<void> {
public:
    static TimeoutFuture dead_timer();
    explicit TimeoutFuture(uint64_t ms);
    futures::PollResult<void> poll(futures::Waker&& waker) override;

private:
    TimeoutFuture();
    bool dead = false;
    bool waker_set = false;
    RcPtr<Mutex<bool>> timed_out = RcPtr(Mutex(false));
    uint64_t tick;
};

}

#endif //WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP
