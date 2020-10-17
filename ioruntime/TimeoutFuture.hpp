//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP
#define WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP

#include "../boxed/RcPtr.hpp"
#include "../futures/IFuture.hpp"
#include "../mutex/mutex.hpp"
#include "GlobalTimeoutEventHandler.hpp"
#include "../option/optional.hpp"

using boxed::RcPtr;
using futures::IFuture;
using mutex::Mutex;
using option::optional;

namespace ioruntime {

class TimeoutFuture : public IFuture<void> {
public:
    TimeoutFuture() = delete;
    TimeoutFuture(TimeoutFuture const&) = delete;
    TimeoutFuture& operator=(TimeoutFuture const&) = delete;
    TimeoutFuture(TimeoutFuture&& other) noexcept;
    TimeoutFuture& operator=(TimeoutFuture&& other) noexcept;
    ~TimeoutFuture() override;
    explicit TimeoutFuture(uint64_t ms);
    futures::PollResult<void> poll(futures::Waker&& waker) override;

private:
    bool _dead = false;
   // bool waker_set = false;
    RcPtr<Mutex<bool>> _timed_out = RcPtr(Mutex(false));
    optional<TimeoutEventConnection> _connection;
    uint64_t _tick;
};

}

#endif //WEBSERV_IORUNTIME_TIMEOUTFUTURE_HPP
