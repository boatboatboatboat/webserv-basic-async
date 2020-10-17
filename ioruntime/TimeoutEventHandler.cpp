//
// Created by Djevayo Pattij on 8/3/20.
//

#include "TimeoutEventHandler.hpp"
#include <sys/time.h>

namespace ioruntime {
uint64_t
TimeoutEventHandler::get_time_ms()
{
    timeval tv { 0, 0 };

    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void TimeoutEventHandler::reactor_step()
{
    auto present_time = get_time_ms();
    std::vector<BoxFunctor> callbacks;

    // First, move the callbacks out of the timeout map.
    {
        auto clocks = clocks_mutex.lock();

        for (auto it = clocks->begin(); it != clocks->end();) {
            auto& timeout = *it;
            if (timeout.first <= present_time) {
                callbacks.push_back(std::move(timeout.second.functor));
                // Mark timeout as used, so we know it has been called.
                timeout.second.used = true;
            } else {
                // check if disconnected
                bool disc;
                {
                    auto ulock = timeout.second.disconnected->lock();
                    disc = *ulock;
                }
                if (disc) {
                    it = clocks->erase(it);
                    continue;
                }
            }
            it++;
        }
    }
    // Now we can safely call the callbacks.
    for (auto& callback : callbacks) {
        (*callback)();
    }
    // Remove the used callbacks
    {
        auto clocks = clocks_mutex.lock();

        auto it = clocks->begin();
        while (it != clocks->upper_bound(present_time)) {
            auto& timeout = *it;
            auto used = timeout.second.used;
            if (used)
                it = clocks->erase(it);
            else
                it++;
        }
    }
}

auto TimeoutEventHandler::register_timeout(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection
{
    auto tick = get_time_ms() + ms;
    auto clocks = clocks_mutex.lock();
    auto uptr = RcPtr(Mutex(false));
    clocks->insert(std::pair(tick, CallbackInfo { .disconnected = RcPtr(uptr), .functor = std::move(callback), .used = false }));
    return TimeoutEventConnection(std::move(uptr));
}

auto TimeoutEventHandler::register_timeout_real(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection
{
    auto clocks = clocks_mutex.lock();
    auto uptr = RcPtr(Mutex(false));
    clocks->insert(std::pair(ms, CallbackInfo { .disconnected = RcPtr(uptr), .functor = std::move(callback), .used = false }));
    return TimeoutEventConnection(std::move(uptr));
}

auto TimeoutEventHandler::is_clocks_empty() -> bool
{
    auto clocks = clocks_mutex.lock();
    return clocks->empty();
}

TimeoutEventConnection::TimeoutEventConnection(RcPtr<Mutex<bool>>&& handle)
    : _handle(std::move(handle))
{
}

TimeoutEventConnection::~TimeoutEventConnection()
{
    disconnect();
}

void TimeoutEventConnection::disconnect()
{
    if (!_disconnected) {
        auto hlock = _handle->lock();
        *hlock = true;
        _disconnected = true;
    }
}

TimeoutEventConnection::TimeoutEventConnection(TimeoutEventConnection&& other) noexcept
    : _handle(std::move(other._handle))
{
    _disconnected = other._disconnected;
    other._disconnected = true;
}

TimeoutEventConnection& TimeoutEventConnection::operator=(TimeoutEventConnection&& other) noexcept
{
    if (this == &other)
        return *this;
    _handle = std::move(other._handle);
    _disconnected = other._disconnected;
    other._disconnected = true;
    return *this;
}

}
