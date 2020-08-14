//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_POOLEDEXECUTOR_HPP
#define WEBSERV_POOLEDEXECUTOR_HPP

#include "../ioruntime/IExecutor.hpp"
#include "ioruntime.hpp"
#include "../futures/futures.hpp"
#include "../mutex/mutex.hpp"
#include <pthread.h>
#include <queue>

using boxed::RcPtr;
using mutex::Mutex;

namespace futures { class Task; } using futures::Task; // forward declaration
using TaskQueue = std::queue<RcPtr<Task>>;

struct WorkerMessage {
    int id;
    std::vector<Mutex<TaskQueue>>* queues;
};

namespace ioruntime {
    // classes
class PooledExecutor : public IExecutor { // C++ bug
public:
    PooledExecutor() = delete;
    PooledExecutor(int worker_count);
    ~PooledExecutor();
    void spawn(RcPtr<Task>&& task) override;
    bool step() override;

private:
    [[noreturn]] static void* worker_thread_function(WorkerMessage* message);
    static bool steal_task(WorkerMessage* message, RcPtr<Task>& task);
    std::vector<pthread_t> workers;
    std::vector<WorkerMessage> messages;
    std::vector<Mutex<TaskQueue>> task_queues;
    Mutex<int> spawn_head;
};
}

#endif // WEBSERV_POOLEDEXECUTOR_HPP
