//
// Created by boat on 7/17/20.
//

#include "RuntimeBuilder.hpp"
#include "PooledExecutor.hpp"
#include "Runtime.hpp"
#include "ThreadlessExecutor.hpp"

namespace ioruntime {

RuntimeBuilder& RuntimeBuilder::with_workers(int d)
{
    pooled = true;
    if (d <= 0) {
        // TODO: make exception
        throw "worker count is zero or less";
    }
    worker_count = d;
    return *this;
}

RuntimeBuilder& RuntimeBuilder::without_workers()
{
    pooled = false;
    worker_count = 1;
    return *this;
}

Runtime RuntimeBuilder::build()
{
    Runtime rt;

    if (pooled) {
        rt.executor = BoxPtr<PooledExecutor>::make(worker_count);
    } else {
        rt.executor = BoxPtr<ThreadlessExecutor>::make();
    }
    return rt;
}

Runtime RuntimeBuilder::dead_runtime() { return Runtime(); }
} // namespace ioruntime
