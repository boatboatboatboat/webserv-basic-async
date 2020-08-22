//
// Created by boat on 11-07-20.
//

#include "Runtime.hpp"
#include "IExecutor.hpp"
#include <iostream>

using ioruntime::IExecutor;

namespace ioruntime {
void Runtime::register_handler(BoxPtr<IEventHandler>&& handler)
{
    handlers.push_back(std::move(handler));
}

void Runtime::register_io_handler(BoxPtr<IoEventHandler>&& handler)
{
    register_handler(std::move(handler));
    GlobalIoEventHandler::set(dynamic_cast<IoEventHandler*>(handlers.back().get()));
}

void Runtime::naive_run()
{
    do {
        // Step all the handlers
        for (auto& handler : handlers) {
            handler->reactor_step();
        }
        // usleep(0) will 'yield' the thread
        // it basically hints the cpu to not spin like crazy
        usleep(0);
    } while (executor->step());
}

Runtime::Runtime() = default;

void Runtime::spawn(BoxPtr<IFuture<void>>&& future)
{
    auto task = RcPtr<Task>::make(std::move(future), executor.get());
    executor->spawn(std::move(task));
}
} // namespace ioruntime
