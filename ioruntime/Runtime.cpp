//
// Created by boat on 11-07-20.
//

#include "Runtime.hpp"
#include "GlobalTimeoutEventHandler.hpp"
#include "IExecutor.hpp"
#include "TimeoutEventHandler.hpp"
#include <iostream>

using ioruntime::IExecutor;

namespace ioruntime {
void Runtime::register_handler(BoxPtr<IEventHandler>&& handler, HandlerType ht = Any)
{
    handlers.push_back(std::move(handler));
    switch (ht) {
    case Io: {
        GlobalIoEventHandler::set(dynamic_cast<IoEventHandler*>(handlers.back().get()));
    } break;
    case Timeout: {
        GlobalTimeoutEventHandler::set(dynamic_cast<TimeoutEventHandler*>(handlers.back().get()));
    } break;
    default: break;
    }
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
