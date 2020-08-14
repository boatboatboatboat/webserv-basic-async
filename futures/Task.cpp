//
// Created by boat on 8/12/20.
//

#include "Task.hpp"

namespace futures {
bool Task::consume(BoxPtr<IFuture<void>>& out)
{
    auto guard = inner_task.lock();
    if (guard->stale) {
        return false;
    } else {
        out = std::move(guard->future);
        guard->stale = true;
    }
    return true;
}

void Task::deconsume(BoxPtr<IFuture<void>>&& out)
{
    auto guard = inner_task.lock();
    guard->stale = false;
    guard->future = std::move(out);
}

Task::Task(BoxPtr<IFuture<void>>&& future, IExecutor* origin):
	inner_task(std::move(Mutex<InnerTask>::make(std::move(future)))),
	task_sender(origin)
{
}

IExecutor* Task::get_sender()
{
    return task_sender;
}

Waker Task::derive_waker()
{
    return Waker(RcPtr(std::move(*this)));
}

Task::InnerTask::InnerTask(BoxPtr<IFuture<void>>&& p_future)
{
    future = std::move(p_future);
    stale = false;
}
}