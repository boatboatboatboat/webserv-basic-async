//
// Created by boat on 12-07-20.
//

#include "ThreadlessExecutor.hpp"
#include <iostream>

namespace ioruntime {
void ThreadlessExecutor::spawn(BoxPtr<IFuture<void>>&& future)
{
    tasks_until_completion += 1;
    tasks.push_back(std::move(future));
}

ThreadlessExecutor::ThreadlessExecutor()
{
    std::cout << "Threadless executor created" << std::endl;
}

bool ThreadlessExecutor::step()
{
    while (!tasks.empty()) {
        Waker waker(std::move(tasks.back()));
        auto result = waker.get_future().poll(std::move(waker));
        if (result.is_ready())
            tasks_until_completion -= 1;
        tasks.pop_back();
    }
    return (tasks_until_completion > 0);
}
} // namespace ioruntime