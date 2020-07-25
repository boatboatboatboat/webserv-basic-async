//
// Created by boat on 6/27/20.
//

#include "mutex.hpp"
#include <iostream>

// Typed mutexguard

namespace mutex {

template <typename T>
MutexGuard<T>::MutexGuard(Mutex<T>& mutex)
    : mutex(mutex)
{
    pthread_mutex_lock(&mutex.get_inner_mutex());
}

template <typename T>
MutexGuard<T>::~MutexGuard()
{
    pthread_mutex_unlock(&mutex.get_inner_mutex());
}

template <typename T>
T& MutexGuard<T>::get()
{
    return mutex.get_inner_type();
}

template <typename T>
T& MutexGuard<T>::operator*()
{
    return mutex.get_inner_type();
}

template <typename T>
T* MutexGuard<T>::operator->()
{
    return &mutex.get_inner_type();
}

template <typename T>
Mutex<T>::Mutex()
{
    int result = pthread_mutex_init(&this->inner_mutex, NULL);

    if (result) {
        // TODO: better mutex errors
        throw "Nice Mutex error!!!";
    }

    this->inner_type = T();
}

template <typename T>
Mutex<T>::Mutex(T inner)
{
    int result = pthread_mutex_init(&this->inner_mutex, NULL);

    if (result) {
        // TODO: make better mutex errors
        throw "Nice Mutex Error!!!";
    }

    this->inner_type = std::move(inner);
}

template <typename T>
Mutex<T>::~Mutex()
{
    /*
         * NOTE:
         * pthread_mutex_destroy can actually error,
         * but we can't error in a destructor.
         * however these errors are related to either having an invalid `inner_mutex`,
         * or still having a lock to the mutex,
         * which we can avoid by just using the mutex guards.
         */
    pthread_mutex_destroy(&this->inner_mutex);
}

template <typename T>
MutexGuard<T> Mutex<T>::lock()
{
    return MutexGuard<T>(*this);
};

template <typename T>
pthread_mutex_t& Mutex<T>::get_inner_mutex()
{
    return this->inner_mutex;
}

template <typename T>
T& Mutex<T>::get_inner_type()
{
    return this->inner_type;
}

}