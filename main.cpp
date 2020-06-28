#include <iostream>
#include <fcntl.h>
#include <zconf.h>
#include <vector>
#include <queue>
#include <pthread.h>
#include "box_ptr.hpp"
#include "mutex.hpp"
#include "rc_ptr.hpp"

template<class T>
class PollResult {
public:
    static PollResult<T> pending() {
        return PollResult<T>();
    }

    static PollResult<T> ready(T result) {
        return PollResult<T>(result);
    }

    bool is_ready() const {
        return _ready;
    }

    T get_result() {
        return _result;
    }
private:
    explicit PollResult(T result) {
        _ready = true;
        _result = result;
    }
    explicit PollResult() {
        _ready = false;
    }
    bool _ready;
    T _result;
};

template<>
class PollResult<void> {
public:
    static PollResult pending() {
        return PollResult<void>(false);
    }

    static PollResult ready() {
        return PollResult<void>(true);
    }

    bool is_ready() const {
        return _ready;
    }

private:
    explicit PollResult(bool ready) {
        _ready = ready;
    };

    bool _ready;
};

template<class T = void>
class IFuture {
public:
    virtual PollResult<T> poll() = 0;
};

template<typename O>
class ThenFuture: public IFuture<void> {
public:
    ThenFuture(IFuture<O>* fut, void (*f)(O)) {
        _future = fut;
        _closure = f;
    }

    PollResult<void> poll() override {
        PollResult<O> result = _future->poll();

        if (result.is_ready()) {
            _closure(result.get_result());
            return PollResult<void>::ready();
        } else {
            return PollResult<void>::pending();
        }
    }
private:
    IFuture<O>* _future;
    void (*_closure)(O);
};

template<>
class ThenFuture<void>: public IFuture<void> {
public:
	ThenFuture(IFuture<void>* fut, void (*f)()) {
		_future = fut;
		_closure = f;
	}

	PollResult<void> poll() override {
		PollResult<void> result = _future->poll();

		if (result.is_ready()) {
			_closure();
			return PollResult<void>::ready();
		} else {
			return PollResult<void>::pending();
		}
	}
private:
	IFuture<void>* _future;
	void (*_closure)(void);
};

template<typename O>
class FutureExt: public IFuture<O> {
public:
    ThenFuture<O> and_then(void (*f)(O)) {
        return ThenFuture<O>(this, f);
    }
};

template<>
class FutureExt<void>: public IFuture<void> {
public:
	ThenFuture<void> and_then(void (*f)()) {
		return ThenFuture<void>(this, f);
	}
};

class GetLine: public FutureExt<std::string> {
public:
    explicit GetLine(int fd) {
        _fd = fd;
    }

    PollResult<std::string> poll() override {
        fcntl(_fd, F_SETFL, O_NONBLOCK);
        fd_set readables;
        timeval lit_zero;

        lit_zero.tv_sec = 0;
        lit_zero.tv_usec = 0;

        FD_ZERO(&readables);
        FD_SET(_fd, &readables);

        select(_fd + 1, &readables, nullptr, nullptr, &lit_zero);
        if (FD_ISSET(_fd, &readables)) {
        	int retv = read(_fd, &_buffer, 12);
        	if (retv == 0) {
				return PollResult<std::string>::ready(ret);
			}

        	for (int k = 0; k < retv; k += 1) {
				if (_buffer[k] == '\n')
					return PollResult<std::string>::ready(ret);
				ret += _buffer[k];
        	}

            return PollResult<std::string>::pending();
        }
        return PollResult<std::string>::pending();
    }
private:
	std::string ret;
    char _buffer[12];
    int _fd;
};

template<typename T>
T block_on(IFuture<T>& future) {
    PollResult<T> poll_result;

    while (!poll_result.is_ready()) {
        poll_result = future.poll();
    }

    return poll_result.get_result();
};

template<typename T>
class IoRuntime;
template<typename T>
class IoRuntimeBuilder;
template<typename T>
class IoRuntimeInfo;

template<typename T>
class WorkerMessage {
public:
	WorkerMessage(int id, RcPtr<IoRuntimeInfo<T>> info): id(id), info(info) {}
	int id;
	RcPtr<IoRuntimeInfo<T>> info;
private:
};

template<typename T>
void	*basic_worker(void *msg) {
	auto& message = *static_cast<WorkerMessage<T>*>(msg);

	std::cout << "Hello from worker " << message.id << " runtime " << &*message.info << std::endl;
	auto& mutex_queue = message.info->get_tasks().at(message.id);

	for (;;) {
		IFuture<T>* fut = nullptr;
		{
			std::queue<IFuture<T>*>& current_queue = mutex_queue.lock().get();

			if (current_queue.empty()) {
				// We don't have any tasks - steal one from another worker
				for (auto& other_mutex_queue: message.info->get_tasks()) {
					auto& other_queue = other_mutex_queue.lock().get();

					// If we find another queue with more than one task, steal it from that queue.
					if (!other_queue.empty()) {
						fut = other_queue.front();
						other_queue.pop();
						std::cout << "A task has been stolen!" << std::endl;
						break;
					}
				}

				if (fut == nullptr) {
					// We couldn't find any other available tasks - yield the thread
					usleep(1000); // TODO: find a better way to yield the thread
					continue;
				}
			} else {
				fut = current_queue.front();
				current_queue.pop();
			}
		}

		if (fut->poll().is_ready()) {
			std::cout << "Task has been ran to completion by " << message.id << std::endl;
		} else {
			std::queue<IFuture<T>*>& current_queue = mutex_queue.lock().get();

			// TODO: instead of requeueing the future, setup a waker system
			current_queue.push(fut);
		}
	}
	return (NULL);
}

template<typename T>
class IoRuntimeBuilder {
public:
	IoRuntimeBuilder& with_thread_count(int i) {
		this->thread_count = i;
		return *this;
	}

	IoRuntime<T> build() {
		RcPtr<IoRuntimeInfo<T>> info;
		IoRuntime<T> runtime(info);

		for (int i = 0; i < this->thread_count; i += 1) {
			WorkerMessage<T> message(i, RcPtr(info));

			info->get_messages().push_back(message);
			info->get_tasks().push_back(Mutex(std::queue<IFuture<T>*>()));
		}
		return runtime;
	}
private:
	int thread_count;
};

template<typename T>
class IoRuntimeInfo {
public:
	IoRuntimeInfo() = default;
	~IoRuntimeInfo() = default;
	IoRuntimeInfo(const IoRuntimeInfo& other) = default;

	std::vector<pthread_t>& get_workers() {
		return workers;
	}

	std::vector<Mutex<std::queue<IFuture<T>*>>>& get_tasks() {
		return tasks;
	}

	std::vector<WorkerMessage<T>>& get_messages() {
		return messages;
	}
private:
	std::vector<Mutex<std::queue<IFuture<T>*>>> tasks;
	std::vector<pthread_t> workers;
	std::vector<WorkerMessage<T>> messages;
};

template<class T>
class IoRuntime {
	friend class IoRuntimeBuilder<T>;
public:
	~IoRuntime() {
		// TODO: write an actual destructor
		std::cout << "warning: no IoRuntime destructor " << __FILE__ << ":" << __LINE__ << std::endl;
	}

    void spawn(IFuture<T>* future) {
		// When we spawn a new task,
		// we give it to the worker with the lowest queued tasks.
		int lowest_task = -1;
		int current_task = 0;
		unsigned long amt;

		for (auto& mutex_queue: info->get_tasks()) {
			std::queue<IFuture<T>*> queue = mutex_queue.lock().get();

			if (lowest_task == -1 || queue.size() < amt) {
				lowest_task = current_task;
				amt = queue.size();
			}
			current_task += 1;
		}

		std::queue<IFuture<T>*>& queue = info->get_tasks().at(lowest_task).lock().get();

		queue.push(future);
		std::cout << "Enqueued a task onto " << lowest_task << ' ' << &queue << std::endl;
    }

    void initialize() {
    	std::cout << "Initializing runtime with " << info->get_tasks().size() << " workers" << std::endl;
    	std::cout << "Runtime pointer: " << &*info << std::endl;
    	for (unsigned long i = 0; i < info->get_tasks().size(); i += 1) {
			pthread_t new_thread;

			if (pthread_create(&new_thread, NULL, &basic_worker<T>, &info->get_messages().at(i))) {
				throw "thread creation failed";
			}

			info->get_workers().push_back(new_thread);
		}
    }
    IoRuntime(const IoRuntime& other) = default;
protected:
	IoRuntime() = default;
	IoRuntime(RcPtr<IoRuntimeInfo<T>> info): info(info) {}
private:
	RcPtr<IoRuntimeInfo<T>> info;
};

class NeverReadyFuture: public FutureExt<void> {
public:
	NeverReadyFuture() = default;
	~NeverReadyFuture() = default;
	PollResult<void> poll() override {
		for (;;); // infinite loop
		return PollResult<void>::pending();
	}
};

int main() {
    GetLine gl_future_1(0);
    GetLine gl_future_2(0);
    GetLine gl_future_3(0);

  	auto runtime = IoRuntimeBuilder<void>()
  			.with_thread_count(2)
  			.build();

  	runtime.initialize();

  	NeverReadyFuture never_ready;

  	runtime.spawn(&never_ready);
  	usleep(10000);
  	auto getline1 = gl_future_1.and_then([](auto const out) {std::cout << "From 1: " << out << std::endl; });
  	auto getline2 = gl_future_2.and_then([](auto const out) {std::cout << "From 2: " << out << std::endl; });
  	auto getline3 = gl_future_3.and_then([](auto const out) {std::cout << "From 3: " << out << std::endl; });
  	runtime.spawn(&getline1);
  	runtime.spawn(&getline2);
  	runtime.spawn(&getline3);
	for (;;);

    return 0;
}
