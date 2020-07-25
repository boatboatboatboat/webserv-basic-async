//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_POOLEDEXECUTOR_HPP
#define WEBSERV_POOLEDEXECUTOR_HPP

#include "ioruntime.hpp"

namespace ioruntime {
class PooledExecutor : public IExecutor {
public:
private:
    std::vector < Mutex<std::queue<BoxPtr<IFuture<void>>>> tasks;
};
}

#endif // WEBSERV_POOLEDEXECUTOR_HPP
