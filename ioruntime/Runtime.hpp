//
// Created by boat on 11-07-20.
//

#ifndef WEBSERV_RUNTIME_HPP
#define WEBSERV_RUNTIME_HPP

#include <vector>
#include "../boxed/BoxPtr.hpp"
#include "EventHandler.hpp"
#include "../futures/futures.hpp"
#include "IExecutor.hpp"

using boxed::BoxPtr;
using futures::IFuture;

namespace ioruntime {
    class Runtime {
        friend class RuntimeBuilder;
    public:
        void register_handler(BoxPtr<IEventHandler>&& handler);
        void spawn(BoxPtr<IFuture<void>>&& future);
        void naive_run();
    private:
        Runtime();
        int running_tasks;
        std::vector<BoxPtr<IEventHandler>> handlers;
        BoxPtr<IExecutor> executor;
    };
}

#endif //WEBSERV_RUNTIME_HPP
