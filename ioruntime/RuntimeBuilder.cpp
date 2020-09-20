//
// Created by boat on 7/17/20.
//

#include "RuntimeBuilder.hpp"
#include "PooledExecutor.hpp"
#include "Runtime.hpp"
#include "ThreadlessExecutor.hpp"
#include "TimeoutEventHandler.hpp"

namespace ioruntime {

RuntimeBuilder& RuntimeBuilder::with_workers(uint64_t d)
{
    pooled = true;
    if (d == 0) {
        throw std::runtime_error("worker count is zero");
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
    rt.register_handler(BoxPtr<IoEventHandler>::make(), Runtime::Io);
    rt.register_handler(BoxPtr<TimeoutEventHandler>::make(), Runtime::Timeout);
    return rt;
}

Runtime RuntimeBuilder::dead_runtime() { return Runtime(); }

} // namespace ioruntime
