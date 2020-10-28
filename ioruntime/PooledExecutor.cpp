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
        task_queues.emplace_back(TaskQueue());
    }

    // create messages
    for (int idx = 0; idx < worker_count; idx += 1) {
        messages.push_back({ .id = idx,
            .queues = &task_queues,
            .tasks_running_mutex = &tasks_running_mutex });
    }

    for (int idx = 0; idx < worker_count; idx += 1) {
        pthread_t thread;

        if (pthread_create(
                &thread,
                nullptr,
                reinterpret_cast<void* (*)(void*)>(worker_thread_function),
                &messages[idx])
            != 0) {
            throw std::runtime_error("PooledExecutor: failed to create workers");
        }

        workers.push_back(thread);
    }
}

PooledExecutor::~PooledExecutor() = default;

void PooledExecutor::spawn(RcPtr<Task>&& future)
{
    size_t head;

    {
        auto spawn_head = spawn_head_mutex.lock();

        *spawn_head += 1;
        if (*spawn_head >= workers.size())
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
    int head;

    {
        auto spawn_head = spawn_head_mutex.lock();

        *spawn_head += 1;
        if (*spawn_head >= workers.size())
            *spawn_head = 0;
        head = *spawn_head;
    }
    {
        auto queue = task_queues.at(head).lock();

        queue->push(std::move(future));
    }
}

auto PooledExecutor::step() -> bool
{
    auto tasks_running = tasks_running_mutex.lock();

    return *tasks_running > 0;
}

[[noreturn]] auto
PooledExecutor::worker_thread_function(WorkerMessage* message) -> void*
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
            // If no task was found, we should sleep for a short while instead.
            usleep(500);
            task_found = steal_task(message, task);
        }

        if (task_found) {
            BoxPtr<IFuture<void>> future_slot(nullptr);

            try {
                auto waker = Waker(RcPtr(task));

                if (task->consume(future_slot)) {
                    auto result = future_slot->poll(std::move(waker));
                    if (result.is_pending()) {
                        task->deconsume(std::move(future_slot));
                    } else {
                        auto tasks_running = message->tasks_running_mutex->lock();
                        *tasks_running -= 1;
                    }
                }
            } catch (std::exception const& e) {
                WARNPRINT("Poll error: " << e.what());
            }
            /*
             * try {
                BoxPtr<IFuture<void>> future_slot(nullptr);
                auto waker = Waker(RcPtr(task));

                auto inner_task = task->get_inner_task().lock();

                if (!inner_task->stale) {
                    future_slot = std::move(inner_task->future);
                    if (future_slot.get() == nullptr) {
                        throw std::runtime_error("Tried to poll null future");
                    }
                    auto result = future_slot->poll(std::move(waker));

                    if (result.is_ready()) {
                        inner_task->stale = true;
                        auto tasks_running = message->tasks_running_mutex->lock();

                        *tasks_running -= 1;
                    } else {
                        inner_task->future = std::move(future_slot);
                    }
                }
            } catch (std::exception& e) {
                WARNPRINT("Poll failed: " << e.what());
                throw std::runtime_error("Poll failed - exiting to prevent spin");
            } */
        }
    }
}

auto PooledExecutor::steal_task(WorkerMessage* message, RcPtr<Task>& task) -> bool
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
            // move task out of the queue,
            // into the WORKER, not the worker queue.
            task = std::move(other_queue->front());
            other_queue->pop();
            return true;
        }
        other_id += 1;
    }
    return false;
}

}