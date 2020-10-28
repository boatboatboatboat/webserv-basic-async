//
// Created by Djevayo Pattij on 8/26/20.
//

#include "SetReadyFunctor.hpp"

SetReadyFunctor::SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source, int meta_fd)
    : ready_mutex(std::move(cr_source))
    , _meta_fd(meta_fd)
{
}

SetReadyFunctor::SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source)
    : ready_mutex(std::move(cr_source))
{
}

void SetReadyFunctor::operator()()
{
    auto cread_guard = ready_mutex->lock();
    if (_meta_fd != -1 && *cread_guard) {
        DBGPRINT("evil " << _meta_fd);
    }
    if (_meta_fd >= 0) {
        TRACEPRINT("make ready:: " << _meta_fd);
    }
    *cread_guard = true;
}

auto SetReadyFunctor::dbg_get_mutex() -> RcPtr<Mutex<bool>>&
{
    return ready_mutex;
}

SetReadyFunctor::~SetReadyFunctor()
{
    if (_meta_fd >= 0) {
        TRACEPRINT("srf destruction:: " << _meta_fd);
    }
}

SetReadyFunctor::SetReadyFunctor(SetReadyFunctor&& other) noexcept
    : ready_mutex(std::move(other.ready_mutex))
    , _meta_fd(other._meta_fd)
{
    other._meta_fd = -1;
}

auto SetReadyFunctor::operator=(SetReadyFunctor&& other) noexcept -> SetReadyFunctor&
{
    if (this == &other) {
        return *this;
    }
    ready_mutex = std::move(other.ready_mutex);
    _meta_fd = other._meta_fd;
    other._meta_fd = -1;
    return *this;
}
