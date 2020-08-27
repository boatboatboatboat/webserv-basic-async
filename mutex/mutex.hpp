//
// Created by boat on 6/27/20.
//

#ifndef ASYNCTEST2_MUTEX_HPP
#define ASYNCTEST2_MUTEX_HPP

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>
#include <zconf.h>

namespace mutex {

template <typename T>
class Mutex;

template <typename T>
class MutexGuard {
public:
    MutexGuard() = delete;
    MutexGuard(MutexGuard&&) = delete;
    MutexGuard(MutexGuard&) = delete;

#ifdef DEBUG_MUTEX
    explicit MutexGuard(Mutex<T>& mutex, const char* fin, int line, const char* fun);
#endif
    explicit MutexGuard(Mutex<T>& mutex);

    ~MutexGuard();

    T& operator*();

    T* operator->();

    T& get();

private:
    Mutex<T>& mutex;
};

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

    template <typename... Args>
    static Mutex<T> make(Args&&... args);

    ~Mutex();

    MutexGuard<T> lock();

#ifdef DEBUG_MUTEX
    MutexGuard<T> dbglock(const char* fin, int line, const char* fun);
#endif

private:
    pthread_mutex_t& get_inner_mutex();

    T& get_inner_type();

    pthread_mutex_t inner_mutex;
#ifdef DEBUG_MUTEX_CANARY

    // Google Sanitizers don't pick up on some underflows/overflows.
    // Whenever EINVAL is thrown while trying to lock/unlock a mutex,
    // it's generally because of an underflow/overflow.
    // If compiled with DEBUG_MUTEX_CANARY, a canary will be added
    // at the beginning and end of the value behind the mutex.
    // If the canaries do not match up with whatever the canary constants
    // are set to, it will print an error and abort.
public:
    static const uint64_t TOP_CANARY_VAL = 0x0123456789ABCDEF;
    volatile uint64_t TOP_CANARY = TOP_CANARY_VAL;
private:
#endif
    T inner_type;
#ifdef DEBUG_MUTEX_CANARY
public:
    static const uint64_t BOT_CANARY_VAL = 0xFEDCBA9876543210;
    volatile uint64_t BOT_CANARY = BOT_CANARY_VAL;
private:
#endif
#ifdef DEBUG_MUTEX
    bool dbg_moved_out = false;
    pthread_mutex_t locked_already_mutex;
    pid_t locked_already = 0;
    const char* filename = "(null)";
    int line = 0;
    const char* funcname = "(null)";
#endif
};

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
