//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_IEXECUTOR_HPP
#define WEBSERV_IEXECUTOR_HPP

#include "../boxed/RcPtr.hpp"

using boxed::RcPtr;

namespace futures {
    class Task;
} using futures::Task;

namespace ioruntime {
// classes
class IExecutor {
public:
    virtual ~IExecutor() = 0;
    virtual void spawn(RcPtr<Task>&& task) = 0;
    virtual bool step() = 0;
};

} // namespace ioruntime

#endif // WEBSERV_IEXECUTOR_HPP
