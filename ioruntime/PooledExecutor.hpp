//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_POOLEDEXECUTOR_HPP
#define WEBSERV_POOLEDEXECUTOR_HPP

#include "ioruntime.hpp"
#include "../mutex/mutex.hpp"
#include <queue>
#include <pthread.h>


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
	static void* worker_thread_function(WorkerMessage* message);
    std::vector<pthread_t>			workers;
    std::vector<WorkerMessage>		messages;
	std::vector<Mutex<TaskQueue>>	task_queues;
};
}

#endif // WEBSERV_POOLEDEXECUTOR_HPP
