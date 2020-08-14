//
// Created by boat on 12-07-20.
//

#include "PooledExecutor.hpp"
#include "../futures/futures.hpp"

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

        // check if we have a task on our own queue
        {
            auto my_queue = my_queue_mutex.lock();
            if (!my_queue->empty()) {
                auto task = my_queue->back();
                BoxPtr<IFuture<void>> future_slot;
                if (task->consume(future_slot)) {
                    auto waker = task->derive_waker();
                    auto result = future_slot->poll(std::move(waker));

                    if (result.is_pending()) {
                        task->deconsume(std::move(future_slot));
                    }
                }
                my_queue->pop();

                continue;
            }
        }

        {
            auto task = RcPtr<Task>::uninitialized();
            if (steal_task(message, task)) {
                BoxPtr<IFuture<void>> future_slot(nullptr);

                if (task->consume(future_slot)) {
                    auto waker = task->derive_waker();
                    auto result = future_slot->poll(std::move(waker));

                    if (result.is_pending()) {
                        std::cout << "The task is pending" << std::endl;
                        task->deconsume(std::move(future_slot));
                    } else {
                        std::cout << "The task was ran to completion" << std::endl;
                    }
                }
            }
        }
    }
}
bool PooledExecutor::steal_task(WorkerMessage* message, RcPtr<Task>& task)
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