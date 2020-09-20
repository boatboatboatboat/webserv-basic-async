//
// Created by boat on 7/17/20.
//

#include "GlobalRuntime.hpp"

namespace ioruntime {
// FIXME: Global ctor can throw
Mutex<Runtime*> GlobalRuntime::runtime(nullptr);

void GlobalRuntime::set_runtime(ioruntime::Runtime* new_runtime)
{
    auto guard = runtime.lock();
    *guard = new_runtime;
}

auto GlobalRuntime::get() -> Mutex<Runtime*>*
{
    return &runtime;
}

void GlobalRuntime::spawn_boxed(BoxPtr<IFuture<void>>&& fut)
{
    auto mut = GlobalRuntime::get();
    auto lock = mut->lock();
    (*lock)->spawn(std::move(fut));
}

} // namespace ioruntime
