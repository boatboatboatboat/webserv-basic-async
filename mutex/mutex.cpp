//
// Created by boat on 8/26/20.
//

#include "mutex.hpp"

namespace mutex {

MutexGuard<void>::MutexGuard(Mutex<void>& mutex): mutex(mutex)
{
    pthread_mutex_lock(&mutex.get_inner_mutex());
}

MutexGuard<void>::~MutexGuard()
{
    pthread_mutex_unlock(&mutex.get_inner_mutex());
}

Mutex<void>::Mutex()
{
    int result = pthread_mutex_init(&inner_mutex, nullptr);

    if (result) {
        std::stringstream err;

        err << "Failed to lock mutex: " << strerror(errno);
        throw std::runtime_error(err.str());
    }
}

Mutex<void>::~Mutex()
{
    pthread_mutex_destroy(&inner_mutex);
}

}