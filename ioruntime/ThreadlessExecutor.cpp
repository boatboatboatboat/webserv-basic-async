//
// Created by boat on 12-07-20.
//

#include "ThreadlessExecutor.hpp"
#include <iostream>

namespace ioruntime {
void ThreadlessExecutor::spawn(BoxPtr<IFuture<void>>&& future)
{
    tasks.push_back(std::move(future));
}

ThreadlessExecutor::ThreadlessExecutor()
{
    std::cout << "new threadless executor" << std::endl;
}

void ThreadlessExecutor::step()
{
    while (!tasks.empty()) {
        Waker waker(std::move(tasks.back()));
        waker.get_future().poll(std::move(waker));
        tasks.pop_back();
    }
}
} // namespace ioruntime