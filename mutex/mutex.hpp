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

    Mutex(const Mutex&) = delete;
    Mutex& operator=(Mutex const&) = delete;
    Mutex(Mutex&& other);
    Mutex& operator=(Mutex&&) = default;

    explicit Mutex(T&& inner);

    template<typename... Args>
    static Mutex<T> make(Args&&... args);

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

    Mutex(const Mutex&) = delete;
    Mutex& operator=(Mutex const&) = delete;

    MutexGuard<void> lock();

private:
    pthread_mutex_t& get_inner_mutex();

    pthread_mutex_t inner_mutex;
};
} // namespace mutex

#include "mutex.ipp"

#endif // ASYNCTEST2_MUTEX_HPP
