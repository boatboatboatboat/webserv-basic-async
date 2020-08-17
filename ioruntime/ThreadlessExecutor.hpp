//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_THREADLESSEXECUTOR_HPP
#define WEBSERV_THREADLESSEXECUTOR_HPP

#include "../boxed/boxed.hpp"
#include "../futures/futures.hpp"
#include "IExecutor.hpp"
#include "ioruntime.hpp"
#include <vector>

using boxed::RcPtr;

namespace futures {
class Task;
}
using futures::Task; // forward declaration

namespace ioruntime {
// forward declarations
//class IExecutor;
// classes
class ThreadlessExecutor : public IExecutor {
public:
    ThreadlessExecutor();
    ~ThreadlessExecutor() = default;
    void spawn(RcPtr<Task>&& task) override;
    void respawn(RcPtr<Task>&& task) override;
    bool step() override;

private:
    int tasks_until_completion = 0;
    std::vector<RcPtr<Task>> tasks;
};
} // namespace ioruntime

#endif // WEBSERV_THREADLESSEXECUTOR_HPP
