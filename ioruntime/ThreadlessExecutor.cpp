//
// Created by boat on 12-07-20.
//

#include "ThreadlessExecutor.hpp"
#include "../futures/futures.hpp"
#include <iostream>

using futures::Task;

namespace ioruntime {
void ThreadlessExecutor::spawn(RcPtr<Task>&& task)
{
    tasks_until_completion += 1;
    tasks.push(std::move(task));
}

void ThreadlessExecutor::respawn(RcPtr<Task>&& task)
{
    tasks.push(std::move(task));
}

ThreadlessExecutor::ThreadlessExecutor()
{
    TRACEPRINT("Threadless executor created");
}

bool ThreadlessExecutor::step()
{
    while (!tasks.empty()) {
        try {
            auto task = tasks.front();
            BoxPtr<IFuture<void>> future_slot(nullptr);
            // create new waker by cloning the RcPtr
            auto waker = Waker(RcPtr(task));
            if (task->consume(future_slot)) { // stale task
                auto result = future_slot->poll(std::move(waker));
                if (result.is_pending()) {
                    task->deconsume(std::move(future_slot)); // unstale task
                } else {
                    tasks_until_completion -= 1;
                }
            }
            tasks.pop();
        } catch (std::exception& e) {
            tasks.pop();
            ERRORPRINT("Poll error: " << e.what());
        }
    }
    return (tasks_until_completion > 0);
}
} // namespace ioruntime