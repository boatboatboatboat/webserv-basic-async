//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_RUNTIMEBUILDER_HPP
#define WEBSERV_RUNTIMEBUILDER_HPP

#include "ioruntime.hpp"
#include "Runtime.hpp"


namespace ioruntime {
class RuntimeBuilder {
public:
    RuntimeBuilder() = default;
    RuntimeBuilder& with_workers(int d);
    RuntimeBuilder& without_workers();
    Runtime build();
    static Runtime dead_runtime();

private:
    bool pooled = false;
    int worker_count = 0;
};

} // namespace ioruntime

#endif // WEBSERV_RUNTIMEBUILDER_HPP
