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

        for (auto it = clocks->begin(); it != clocks->upper_bound(present_time); it++) {
            auto& timeout = *it;
            callbacks.push_back(std::move(timeout.second.functor));
            // Mark timeout as used, so we know it has been called.
            timeout.second.used = true;
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
            if (timeout.second.used)
                it = clocks->erase(it);
            else
                it++;
        }
    }
}

void TimeoutEventHandler::register_timeout(uint64_t ms, BoxFunctor&& callback)
{
    auto tick = get_time_ms() + ms;
    auto clocks = clocks_mutex.lock();
    clocks->insert(std::pair(tick, CallbackInfo { .used = false, .functor = std::move(callback) }));
}

void TimeoutEventHandler::register_timeout_real(uint64_t ms, BoxFunctor&& callback)
{
    auto clocks = clocks_mutex.lock();
    clocks->insert(std::pair(ms, CallbackInfo { .used = false, .functor = std::move(callback) }));
}

}
