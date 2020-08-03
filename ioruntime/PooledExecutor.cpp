//
// Created by boat on 12-07-20.
//

#include "PooledExecutor.hpp"

namespace ioruntime {

using mutex::Mutex;

PooledExecutor::PooledExecutor(int worker_count)
{
    // create queues
    for (int idx = 0; idx < worker_count; idx += 1) {
        auto queue = TaskQueue();
        auto mutexed_queue = Mutex(std::move(queue));

        task_queues.push_back(std::move(mutexed_queue));
    }

    // create messages
    for (int idx = 0; idx < worker_count; idx += 1) {
        WorkerMessage message;

        message.id = idx;
        message.queues = &task_queues;
        messages.push_back(message);
    }

    for (int idx = 0; idx < worker_count; idx += 1) {
        pthread_t thread;

        if (pthread_create(
                &thread,
                nullptr,
                reinterpret_cast<void* (*)(void*)>(worker_thread_function),
                &messages[idx])
            != 0) {
            // todo: make better exception
            throw "FUCK TTHREADS";
        }
    }
}

PooledExecutor::~PooledExecutor()
{
}

void PooledExecutor::spawn(BoxPtr<IFuture<void>>&& future)
{
    int head = 0;

    {
        auto guard = spawn_head.lock();

        *guard += 1;
        if (*guard > workers.size())
            *guard = 0;
        head = *guard;
    }

    auto queue = task_queues.at(head).lock();

    queue->push(std::move(future));
}

bool PooledExecutor::step()
{
    return true;
}

[[noreturn]] void*
PooledExecutor::worker_thread_function(WorkerMessage* message)
{
    for (;;) {
        auto& my_queue_mutex = message->queues->at(message->id);
        BoxPtr<IFuture<void>> task(nullptr);
        bool task_found;

        // check if we have a task on our own queue
        {
            auto my_queue = my_queue_mutex.lock();
            task_found = !my_queue->empty();
            if (task_found) {
                task = std::move(my_queue->back());
                my_queue->pop();
            }
        }

        // if no task is found - try to steal a task
        if (!task_found) {
            task_found = steal_task(message, task);
        }

        // check if any tasks were found
        if (task_found) {
            Waker waker(std::move(task));
            auto result = waker.get_future().poll(std::move(waker));
            if (result.is_ready())
                std::cout << "Task ran to completion" << std::endl;
            else
                std::cout << "Task is pending" << std::endl;
        }
    }
}
bool PooledExecutor::steal_task(WorkerMessage* message, BoxPtr<IFuture<void>>& task)
{
    int other_id = 0;
    for (auto& other_queue_mutex : *message->queues) {
        // we already found out we are empty...
        if (message->id == other_id) {
            other_id += 1;
            continue;
        }

        auto other_queue = other_queue_mutex.lock();
        if (!other_queue->empty()) {
            // move task out of the queue
            task = std::move(other_queue->back());
            other_queue->pop();
            return true;
        }
        other_id += 1;
    }
    return false;
}

}