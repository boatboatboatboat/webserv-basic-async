//
// Created by Djevayo Pattij on 8/26/20.
//

#include "SetReadyFunctor.hpp"

SetReadyFunctor::SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source)
    : ready_mutex(std::move(cr_source))
{
}

void SetReadyFunctor::operator()()
{
    auto cread_guard = ready_mutex->lock();
    *cread_guard = true;
}

auto SetReadyFunctor::dbg_get_mutex() -> RcPtr<Mutex<bool>>&
{
    return ready_mutex;
}
