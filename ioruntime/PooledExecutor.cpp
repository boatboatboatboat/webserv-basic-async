//
// Created by boat on 12-07-20.
//

#include "PooledExecutor.hpp"
#include "../futures/futures.hpp"
#include "../utils/utils.hpp"

using futures::Task;

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
        message.tasks_running_mutex = &tasks_running_mutex;
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

void PooledExecutor::spawn(RcPtr<Task>&& future)
{
    size_t head = 0;

    {
        auto spawn_head = spawn_head_mutex.lock();

        *spawn_head += 1;
        if (*spawn_head > workers.size())
            *spawn_head = 0;
        head = *spawn_head;
    }
    {
        auto queue = task_queues.at(head).lock();

        queue->push(std::move(future));
    }
    {
        auto tasks_running = tasks_running_mutex.lock();

        *tasks_running += 1;
    }
}

void PooledExecutor::respawn(RcPtr<Task>&& future)
{
    int head = 0;

    {
        auto spawn_head = spawn_head_mutex.lock();

        *spawn_head += 1;
        if (*spawn_head > workers.size())
            *spawn_head = 0;
        head = *spawn_head;
    }
    {
        auto queue = task_queues.at(head).lock();

        queue->push(std::move(future));
    }
}

bool PooledExecutor::step()
{
    auto tasks_running = tasks_running_mutex.lock();

    // fixme: worker threads can outlive the executor
    return *tasks_running > 0;
}

[[noreturn]] void*
PooledExecutor::worker_thread_function(WorkerMessage* message)
{
    for (;;) {
        // usleep(0) hints the cpu to halt
        usleep(0);

        auto& my_queue_mutex = message->queues->at(message->id);

        // A future poll can take a lock on one of the queues by waker,
        // causing a double lock.
        // The task should be moved out of the queue and then polled.
        auto task = RcPtr<Task>::uninitialized();
        bool task_found;
        {
            auto my_queue = my_queue_mutex.lock();
            task_found = !my_queue->empty();
            if (task_found) {
                task = std::move(my_queue->front());
                my_queue->pop();
            }
        }

        if (!task_found) {
            // If no task was found
            task_found = steal_task(message, task);
        }

        if (task_found) {
            BoxPtr<IFuture<void>> future_slot(nullptr);
            auto waker = Waker(RcPtr(task));

            auto inner_task = task->get_inner_task().lock();

            if (!inner_task->stale) {
                future_slot = std::move(inner_task->future);
                auto result = future_slot->poll(std::move(waker));

                if (result.is_ready()) {
                    inner_task->stale = true;
                    auto tasks_running = message->tasks_running_mutex->lock();

                    *tasks_running -= 1;
                } else {
                    inner_task->future = std::move(future_slot);
                }
            }
        }
    }
}

bool PooledExecutor::steal_task(WorkerMessage* message, RcPtr<Task>& task)
{
    int other_id = 0;
    for (auto& other_queue_mutex : *(message->queues)) {
        // we already found out we are empty...
        if (message->id == other_id) {
            other_id += 1;
            continue;
        }

        auto other_queue = other_queue_mutex.lock();
        if (!other_queue->empty()) {
            // move task out of the queue
            task = std::move(other_queue->front());
            other_queue->pop();
            return true;
        }
        other_id += 1;
    }
    return false;
}

}