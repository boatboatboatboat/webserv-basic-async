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
    tasks.push_back(std::move(task));
}

ThreadlessExecutor::ThreadlessExecutor()
{
    std::cout << "Threadless executor created" << std::endl;
}

bool ThreadlessExecutor::step()
{
    while (!tasks.empty()) {
        auto task = tasks.back();
        BoxPtr<IFuture<void>> future_slot(nullptr);
        if (task->consume(future_slot)) {
            auto waker = task->derive_waker();
            auto result = future_slot->poll(std::move(waker));

            if (result.is_pending()) {
                task->deconsume(std::move(future_slot));
            }
        }
        tasks.pop_back();
    }
    return (tasks_until_completion > 0);
}
} // namespace ioruntime