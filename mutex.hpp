//
// Created by boat on 6/27/20.
//

#ifndef ASYNCTEST2_MUTEX_HPP
#define ASYNCTEST2_MUTEX_HPP

#include <pthread.h>

template<typename T>
class Mutex;

template<typename T>
class MutexGuard {
public:
	MutexGuard(Mutex<T>& mutex);
	~MutexGuard();
	T& get();
private:
	MutexGuard() = delete;
	Mutex<T>& mutex;
};

template<typename T>
MutexGuard<T>::MutexGuard(Mutex<T>& mutex):
	mutex(mutex)
{
	pthread_mutex_lock(&mutex.get_inner_mutex());
}

template<typename T>
MutexGuard<T>::~MutexGuard()
{
	pthread_mutex_unlock(&mutex.get_inner_mutex());
}

template<typename T>
T& MutexGuard<T>::get() {
	return mutex.get_inner_type();
};

template<typename T>
class Mutex {
public:
	Mutex(T inner) {
		int	result = pthread_mutex_init(&this->inner_mutex, NULL);

		if (result) {
			// TODO: make better mutex errors
			throw "Nice Mutex Error!!!";
		}

		this->inner_type = inner;
	}
	~Mutex() {
		/*
		 * NOTE:
		 * pthread_mutex_destroy can actually error,
		 * but we can't error in a destructor.
		 * however these errors are related to either having an invalid `inner_mutex`,
		 * or still having a lock to the mutex,
		 * which we can avoid by just using the mutexguards
		 */
		pthread_mutex_destroy(&this->inner_mutex);
	}
	MutexGuard<T> lock() {
		return MutexGuard<T>(*this);
	};

	pthread_mutex_t& get_inner_mutex() {
		return this->inner_mutex;
	}

	T& get_inner_type() {
		return this->inner_type;
	}
private:
	Mutex() = delete;
	pthread_mutex_t inner_mutex;
	T inner_type;
};

#endif //ASYNCTEST2_MUTEX_HPP
