//
// Created by boat on 8/12/20.
//

#ifndef WEBSERV_TASK_HPP
#define WEBSERV_TASK_HPP

#include "../ioruntime/IExecutor.hpp"
#include "../boxed/BoxPtr.hpp"
#include "../futures/futures.hpp"
#include "../mutex/mutex.hpp"

using boxed::BoxPtr;
using futures::Waker;
using mutex::Mutex;
using ioruntime::IExecutor;

namespace futures {
class Task {
public:
    Task() = delete;
    Task(BoxPtr<IFuture<void>>&& future, IExecutor* origin);
    bool consume(BoxPtr<IFuture<void>>& out);
    void deconsume(BoxPtr<IFuture<void>>&& out);
    Waker derive_waker();
    IExecutor* get_sender();

private:
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
        bool stale;
    };

    Mutex<InnerTask> inner_task;
    IExecutor* task_sender;
};
}

#endif //WEBSERV_TASK_HPP
