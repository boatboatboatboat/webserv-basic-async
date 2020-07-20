//
// Created by boat on 7/17/20.
//

#include "GlobalRuntime.hpp"

namespace ioruntime {
Mutex<Runtime*> GlobalRuntime::runtime(nullptr);

void GlobalRuntime::set_runtime(ioruntime::Runtime* new_runtime)
{
    *runtime.lock() = new_runtime;
}

MutexGuard<Runtime*> GlobalRuntime::get()
{
    auto guard = runtime.lock();
    if (*guard == nullptr) {
        // TODO: make exception
        throw "Runtime has not been set";
    }
    return guard;
}

    void GlobalRuntime::spawn(BoxPtr<IFuture<void>> &&fut) {
        auto guard = GlobalRuntime::get();
        (*guard)->spawn(std::move(fut));
    }
} // namespace ioruntime
