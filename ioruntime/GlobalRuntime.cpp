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

MutexGuard<Runtime*>&& GlobalRuntime::get()
{
    return std::move(runtime.lock());
}
} // namespace ioruntime
