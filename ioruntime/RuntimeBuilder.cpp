//
// Created by boat on 7/17/20.
//

#include "RuntimeBuilder.hpp"
#include "Runtime.hpp"
#include "PooledExecutor.hpp"
#include "ThreadlessExecutor.hpp"

namespace ioruntime {

    RuntimeBuilder &RuntimeBuilder::with_workers(int d) {
        pooled = true;
        if (d <= 0) {
            // TODO: make exception
            throw "worker count is zero or less";
        }
        worker_count = d;
        return *this;
    }

    RuntimeBuilder &RuntimeBuilder::without_workers() {
        pooled = false;
        worker_count = 1;
        return *this;
    }

    Runtime RuntimeBuilder::build() {
        Runtime rt;

        if (pooled) {
            rt.executor = std::move(BoxPtr<PooledExecutor>());
        } else {
            rt.executor = std::move(BoxPtr<ThreadlessExecutor>());
        }
        return rt;
    }
}
