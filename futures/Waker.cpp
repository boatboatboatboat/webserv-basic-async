//
// Created by boat on 7/3/20.
//

#include "Waker.hpp"
#include "../ioruntime/ioruntime.hpp"
#include <iostream>

using ioruntime::GlobalRuntime;

namespace futures {
void Waker::operator()()
{
    task->get_sender()->spawn(RcPtr(task));
}

RcPtr<Task>& Waker::get_task() { return task; }

Waker::Waker(RcPtr<Task>&& future) { *task = std::move(*future); }

Waker::Waker(Waker& other)
{
    task = other.task;
}
} // namespace futures