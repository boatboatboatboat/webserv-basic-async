//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_POOLEDEXECUTOR_HPP
#define WEBSERV_POOLEDEXECUTOR_HPP

#include "../mutex/mutex.hpp"
#include "ioruntime.hpp"
#include <pthread.h>
#include <queue>

using TaskQueue = std::queue<BoxPtr<IFuture<void>>>;

struct WorkerMessage {
    int id;
    std::vector<Mutex<TaskQueue>>* queues;
};

namespace ioruntime {
class PooledExecutor : public IExecutor {
public:
    PooledExecutor() = delete;
    PooledExecutor(int worker_count);
    ~PooledExecutor();
    void spawn(BoxPtr<IFuture<void>>&& future) override;
    bool step() override;

private:
    [[noreturn]] static void* worker_thread_function(WorkerMessage* message);
    static bool steal_task(WorkerMessage* message, BoxPtr<IFuture<void>>& task);
    std::vector<pthread_t> workers;
    std::vector<WorkerMessage> messages;
    std::vector<Mutex<TaskQueue>> task_queues;
    Mutex<int> spawn_head;
};
}

#endif // WEBSERV_POOLEDEXECUTOR_HPP
