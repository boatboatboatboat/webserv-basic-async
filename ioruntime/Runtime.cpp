//
// Created by boat on 11-07-20.
//

#include "Runtime.hpp"

namespace ioruntime {
    void Runtime::register_handler(BoxPtr<IEventHandler> &&handler) {
        handlers.push_back(std::move(handler));
    }

    void Runtime::naive_run() {
        while (running_tasks) {
            for (auto& handler : handlers) {
                handler->reactor_step();
            }
            executor->step();
        }
    }

    Runtime::Runtime() {
        running_tasks = 0;
    }

    void Runtime::spawn(BoxPtr<IFuture<void>> &&future) {
        executor->spawn(std::move(future));
    }
}
