//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_RUNTIMEBUILDER_HPP
#define WEBSERV_RUNTIMEBUILDER_HPP

#include "IExecutor.hpp"
#include "Runtime.hpp"

namespace ioruntime {
    class RuntimeBuilder {
    public:
        RuntimeBuilder() = default;
        RuntimeBuilder& with_workers(int d);
        RuntimeBuilder& without_workers();
        Runtime build();
    private:
        bool pooled = false;
        int worker_count = 0;
    };
}

#endif //WEBSERV_RUNTIMEBUILDER_HPP
