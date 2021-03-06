//
// Created by boat on 8/12/20.
//

#ifndef WEBSERV_TASK_HPP
#define WEBSERV_TASK_HPP

#include "../boxed/BoxPtr.hpp"
#include "../futures/futures.hpp"
#include "../ioruntime/IExecutor.hpp"
#include "../mutex/mutex.hpp"

using boxed::BoxPtr;
using futures::Waker;
using ioruntime::IExecutor;
using mutex::Mutex;

namespace futures {
class Task {
public:
    class InnerTask {
    public:
        InnerTask() = delete;
        // the default move ctor is fine
        // considering future is a BoxPtr.
        InnerTask(InnerTask&&) = default;
        InnerTask& operator=(InnerTask&&) = default;
        ~InnerTask() = default;
        InnerTask(BoxPtr<IFuture<void>>&& p_future);
        BoxPtr<IFuture<void>> future;
        bool stale = false;
        bool in_use = false;
    };

    Task() = delete;
    Task(BoxPtr<IFuture<void>>&& future, IExecutor* origin);
    bool consume(BoxPtr<IFuture<void>>& out);
    void deconsume(BoxPtr<IFuture<void>>&& out);
    void abandon();
    Waker derive_waker();
    IExecutor* get_sender();
    Mutex<futures::Task::InnerTask>& get_inner_task();
    bool is_stale();

    bool in_use();

private:
    Mutex<InnerTask> inner_task;
    IExecutor* task_sender;
};
}

#endif //WEBSERV_TASK_HPP
