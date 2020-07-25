//
// Created by boat on 6/27/20.
//

#ifndef ASYNCTEST2_MUTEX_HPP
#define ASYNCTEST2_MUTEX_HPP

#include <pthread.h>

namespace mutex {

template <typename T>
class Mutex;

template <typename T>
class MutexGuard {
public:
    MutexGuard() = delete;

    explicit MutexGuard(Mutex<T>& mutex);

    ~MutexGuard();

    T& operator*();

    T* operator->();

    T& get();

private:
    Mutex<T>& mutex;
};

// TODO: implement void mutexguard
template <>
class MutexGuard<void> {
public:
    MutexGuard() = delete;

    explicit MutexGuard(Mutex<void>& mutex);

    ~MutexGuard();

private:
    Mutex<void>& mutex;
};

template <typename T>
class Mutex {
    friend class MutexGuard<T>;

public:
    Mutex();

    Mutex(const Mutex&) = default;

    Mutex& operator=(Mutex const&) = default;

    explicit Mutex(T inner);

    ~Mutex();

    MutexGuard<T> lock();

private:
    pthread_mutex_t& get_inner_mutex();

    T& get_inner_type();

    pthread_mutex_t inner_mutex;
    T inner_type;
};

// TODO: implement void mutex
template <>
class Mutex<void> {
    friend class MutexGuard<void>;

public:
    Mutex();

    ~Mutex();

    Mutex(const Mutex&) = default;

    Mutex& operator=(Mutex const&) = default;

    MutexGuard<void> lock();

private:
    pthread_mutex_t& get_inner_mutex();

    pthread_mutex_t inner_mutex;
};
} // namespace mutex

#include "mutex.ipp"

#endif // ASYNCTEST2_MUTEX_HPP
