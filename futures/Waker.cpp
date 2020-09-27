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
    if (!_dead)
        task->get_sender()->respawn(RcPtr(task));
}

RcPtr<Task>& Waker::get_task() { return task; }

Waker::Waker(RcPtr<Task>&& future)
{
    new (&task) RcPtr<Task>(std::move(future));
}

Waker::Waker(Waker& other)
{
    new (&task) RcPtr<Task>(other.task);
}

BoxFunctor
Waker::boxed()
{
    return BoxPtr<Waker>(new Waker(*this));
}

Waker::Waker(dead_waker_t)
    : _dead(true)
{
}

auto Waker::dead() -> Waker
{
    return Waker(dead_waker);
}

Waker::~Waker()
{
    if (!_dead) {
        task.~RcPtr<Task>();
    }
}
} // namespace futures