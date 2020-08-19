//
// Created by boat on 8/12/20.
//

#include "Task.hpp"

namespace futures {
bool Task::consume(BoxPtr<IFuture<void>>& out)
{
    auto guard = inner_task.lock();
    if (guard->in_use)
        return false;
    if (!guard->stale) {
        out = std::move(guard->future);
        // the future is stale for whatever reason
        if (out.get() == nullptr) {
            return false;
        }
        guard->stale = true;
        guard->in_use = true;
    }
    return true;
}

void Task::deconsume(BoxPtr<IFuture<void>>&& out)
{
    auto guard = inner_task.lock();
    guard->stale = false;
    guard->future = std::move(out);
    guard->in_use = false;
}

void Task::abandon()
{
    auto guard = inner_task.lock();
    guard->in_use = false;
}

Task::Task(BoxPtr<IFuture<void>>&& future, IExecutor* origin)
    : inner_task(Mutex<InnerTask>::make(std::move(future)))
    , task_sender(origin)
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
bool Task::is_stale()
{
    auto x = inner_task.lock();
    return x->stale;
}

bool Task::in_use()
{
    auto x = inner_task.lock();
    return x->in_use;
}

Mutex<Task::InnerTask>& Task::get_inner_task()
{
    return this->inner_task;
}

Task::InnerTask::InnerTask(BoxPtr<IFuture<void>>&& p_future)
{
    future = std::move(p_future);
    stale = false;
    in_use = false;
}
}