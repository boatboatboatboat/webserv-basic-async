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
		//auto wtask = RcPtr(task);//->derive_waker(); // derive from stale task!
		auto waker = Waker(RcPtr(task));
		if (task->consume(future_slot)) { // stale task
			auto result = future_slot->poll(std::move(waker));
            if (result.is_pending()) {
                std::cout << "The future is pending" << std::endl;
                task->deconsume(std::move(future_slot)); // unstale task
            }
        } else {
        	std::cout << "Failed to consume because the task is stale" << std::endl;
        }
        tasks.pop_back();
    }
    return (tasks_until_completion > 0);
}
} // namespace ioruntime