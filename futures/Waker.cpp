//
// Created by boat on 7/3/20.
//

#include "Waker.hpp"
#include "../ioruntime/ioruntime.hpp"
#include <iostream>

using ioruntime::GlobalRuntime;
using ioruntime::IExecutor;

namespace futures {
void Waker::operator()()
{
    if (task)
        (*task)->get_sender()->respawn(RcPtr(*task));
}

RcPtr<Task>& Waker::get_task() { return *task; }

Waker::Waker(RcPtr<Task>&& future)
    : task(std::move(future))
{
}

Waker::Waker(Waker& other)
{
    if (other.task) {
        task = RcPtr(*other.task);
    } else {
        task = option::nullopt;
    }
}

BoxFunctor
Waker::boxed()
{
    return BoxPtr<Waker>(new Waker(*this));
}

Waker::Waker(dead_waker_t):
    task(option::nullopt)
{
}

auto Waker::dead() -> Waker
{
    return Waker(dead_waker);
}

} // namespace futures