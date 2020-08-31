//
// Created by boat on 11-07-20.
//

#ifndef WEBSERV_RUNTIME_HPP
#define WEBSERV_RUNTIME_HPP

#include "../boxed/BoxPtr.hpp"
#include "../futures/futures.hpp"
#include "ioruntime.hpp"
#include <vector>

using boxed::BoxPtr;
using futures::IFuture;

namespace ioruntime {

class Runtime {
    friend class RuntimeBuilder;

public:
    enum HandlerType {
        Any,
        Io,
        Timeout
    };
    void register_handler(BoxPtr<IEventHandler>&& handler, HandlerType ht);
    void spawn(BoxPtr<IFuture<void>>&& future);
    void naive_run();
    void globalize();

private:
    Runtime();
    std::vector<BoxPtr<IEventHandler>> handlers;
    BoxPtr<IExecutor> executor;
};
} // namespace ioruntime

#endif // WEBSERV_RUNTIME_HPP
