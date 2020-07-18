//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_IEXECUTOR_HPP
#define WEBSERV_IEXECUTOR_HPP

#include "../futures/futures.hpp"

namespace ioruntime {
class IExecutor {
public:
    virtual ~IExecutor() = 0;
    virtual void spawn(BoxPtr<IFuture<void>>&& future) = 0;
    virtual void step() = 0;
};

} // namespace ioruntime

#endif // WEBSERV_IEXECUTOR_HPP
