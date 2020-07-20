//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_THREADLESSEXECUTOR_HPP
#define WEBSERV_THREADLESSEXECUTOR_HPP

#include "ioruntime.hpp"

namespace ioruntime {
class ThreadlessExecutor : public IExecutor {
public:
    ThreadlessExecutor();
    ~ThreadlessExecutor() = default;
    void spawn(BoxPtr<IFuture<void>>&& future) override;
    bool step() override;

private:
    int tasks_until_completion = 0;
    std::vector<BoxPtr<IFuture<void>>> tasks;
};
} // namespace ioruntime

#endif // WEBSERV_THREADLESSEXECUTOR_HPP
