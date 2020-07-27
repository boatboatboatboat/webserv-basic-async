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
		pthread_t	thread;

		if (pthread_create(
			&thread,
			nullptr,
			reinterpret_cast<void *(*)(void *)>(worker_thread_function),
			&messages[idx]) != 0)
		{
			// todo: make better exception
			throw "FUCK TTHREADS";
		}

	}
}

PooledExecutor::~PooledExecutor()
{

}

void
PooledExecutor::spawn(BoxPtr<IFuture<void>> &&future)
{

}
bool
PooledExecutor::step()
{
	return false;
}
void *
PooledExecutor::worker_thread_function(WorkerMessage *message)
{
	return nullptr;
}
}