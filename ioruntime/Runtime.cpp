//
// Created by boat on 11-07-20.
//

#include "Runtime.hpp"
#include "ioruntime.hpp"
#include <iostream>

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
        for (auto& handler : handlers) {
            handler->reactor_step();
        }
    } while (executor->step());
}

Runtime::Runtime()
{
    std::cout << "Runtime created" << std::endl;
}

void Runtime::spawn(BoxPtr<IFuture<void>>&& future)
{
    executor->spawn(std::move(future));
}
} // namespace ioruntime
