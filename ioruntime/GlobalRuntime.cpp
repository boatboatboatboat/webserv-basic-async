//
// Created by boat on 7/17/20.
//

#include "GlobalRuntime.hpp"

namespace ioruntime {
Mutex<Runtime*> GlobalRuntime::runtime(nullptr);

void GlobalRuntime::set_runtime(ioruntime::Runtime* new_runtime)
{
    auto guard = runtime.lock();
    *guard = new_runtime;
}

Mutex<Runtime*>* GlobalRuntime::get()
{
    return &runtime;
}

void GlobalRuntime::spawn(BoxPtr<IFuture<void>>&& fut)
{
    auto mut = GlobalRuntime::get();
    auto lock = mut->lock();
    (*lock)->spawn(std::move(fut));
}

} // namespace ioruntime
