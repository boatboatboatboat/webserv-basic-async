//
// Created by boat on 6/27/20.
//

#include "../utils/utils.hpp"
#include "mutex.hpp"
#include <iostream>
#include <strings.h>

#include <string>
#include <sys/types.h>
#include <zconf.h>

// Typed mutexguard

namespace mutex {

template <typename T>
MutexGuard<T>::MutexGuard(Mutex<T>& mutex)
    : mutex(mutex)
{
#ifdef DEBUG_MUTEX_CANARY
    if (mutex.TOP_CANARY != mutex.TOP_CANARY_VAL) {
        ERRORPRINT("Mutex top canary died!");
        exit(6);
    }
    if (mutex.BOT_CANARY != mutex.BOT_CANARY_VAL) {
        ERRORPRINT("Mutex bot canary died!");
        exit(6);
    }
#endif
    int res = pthread_mutex_lock(&mutex.get_inner_mutex());
    if (res) {
        std::stringstream errmsg;

        // fun fact: pthread_mutex_lock does not set errno
        // instead it returns the error value
        errmsg << "MutexGuard: Failed to lock mutex: " << strerror(res);
        throw std::runtime_error(errmsg.str());
    }
#ifdef DEBUG_MUTEX
    pthread_mutex_lock(&mutex.locked_already_mutex);
    mutex.locked_already = gettid();
    pthread_mutex_unlock(&mutex.locked_already_mutex);
#endif
}

template <typename T>
MutexGuard<T>::~MutexGuard()
{
#ifdef DEBUG_MUTEX
    pthread_mutex_lock(&mutex.locked_already_mutex);
#endif
#ifdef DEBUG_MUTEX_CANARY
    if (mutex.TOP_CANARY != mutex.TOP_CANARY_VAL) {
        ERRORPRINT("Mutex top canary died!");
        exit(6);
    }
    if (mutex.BOT_CANARY != mutex.BOT_CANARY_VAL) {
        ERRORPRINT("Mutex bot canary died!");
        exit(6);
    }
#endif
    int res = pthread_mutex_unlock(&mutex.get_inner_mutex());
    if (res) {
        std::stringstream errmsg;

        errmsg << "MutexGuard: Failed to unlock mutex: " << strerror(res);
        DBGPRINT(errmsg.str());
    }
#ifdef DEBUG_MUTEX
    mutex.locked_already = 0;
    mutex.filename = "(null)";
    mutex.line = 0;
    mutex.funcname = "(null)";
    pthread_mutex_unlock(&mutex.locked_already_mutex);
#endif
}

#ifdef DEBUG_MUTEX
template <typename T>
MutexGuard<T>::MutexGuard(Mutex<T>& mutex, const char* fin, int line, const char* fun)
    : mutex(mutex)
{
    std::stringstream fuck;
    pid_t tid = syscall(SYS_gettid);
    fuck << "LockAcq: " << &mutex.get_inner_mutex() << " from " << tid;
    //        DBGPRINT(fuck.str());

    int res = pthread_mutex_lock(&mutex.get_inner_mutex());
    if (res) {
        std::stringstream fuck;
        fuck << "Mutex lock failure " << strerror(errno);
        DBGPRINT(fuck.str());
    }
    pthread_mutex_lock(&mutex.locked_already_mutex);
    mutex.locked_already = gettid();
    mutex.filename = fin;
    mutex.line = line;
    mutex.funcname = fun;
    pthread_mutex_unlock(&mutex.locked_already_mutex);
}
#endif

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
#ifdef DEBUG_MUTEX_CANARY
    if (TOP_CANARY != TOP_CANARY_VAL) {
        ERRORPRINT("Mutex top canary died!");
        exit(6);
    }
    if (BOT_CANARY != BOT_CANARY_VAL) {
        ERRORPRINT("Mutex bot canary died!");
        exit(6);
    }
#endif
#ifdef DEBUG_MUTEX

    pthread_mutex_init(&this->locked_already_mutex, NULL);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    int result = pthread_mutex_init(&this->inner_mutex, NULL);

#else

    int result = pthread_mutex_init(&this->inner_mutex, NULL);

#endif

    if (result) {
        throw std::runtime_error("Mutex init failed");
    }

    this->inner_type = T();
}

template <typename T>
Mutex<T>::Mutex(T&& inner)
    : inner_type(std::move(inner))
{
#ifdef DEBUG_MUTEX_CANARY
    if (TOP_CANARY != TOP_CANARY_VAL) {
        ERRORPRINT("Mutex top canary died!");
        exit(6);
    }
    if (BOT_CANARY != BOT_CANARY_VAL) {
        ERRORPRINT("Mutex bot canary died!");
        exit(6);
    }
#endif
#ifdef DEBUG_MUTEX
    pthread_mutex_init(&this->locked_already_mutex, NULL);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    int result = pthread_mutex_init(&this->inner_mutex, NULL);

#else

    int result = pthread_mutex_init(&this->inner_mutex, NULL);

#endif

    if (result) {
        // we have to recover from the initializer move.
        inner = std::move(inner_type);
        throw std::runtime_error("Mutex init failed");
    }
}

template <typename T>
Mutex<T>::~Mutex()
{
#ifdef DEBUG_MUTEX_CANARY
    if (TOP_CANARY != TOP_CANARY_VAL) {
        ERRORPRINT("Mutex top canary died!");
        exit(6);
    }
    if (BOT_CANARY != BOT_CANARY_VAL) {
        ERRORPRINT("Mutex bot canary died!");
        exit(6);
    }
#endif
    /*
         * NOTE:
         * pthread_mutex_destroy can actually error,
         * but we can't error in a destructor.
         * however these errors are related to either having an invalid `inner_mutex`,
         * or still having a lock to the mutex,
         * which we can avoid by just using the mutex guards.
         */
    int err = pthread_mutex_destroy(&this->inner_mutex);
    (void)err;
#ifdef DEBUG_MUTEX
    if (err) {
        std::stringstream errmsg;

        errmsg << "Mutex[" << &this->inner_mutex << "] destruction failed: " << strerror(err);
        DBGPRINT(errmsg.str());
    } else {
        std::stringstream errmsg;

        errmsg << "Mutex[" << &this->inner_mutex << "] destruction SUCCESS";
        DBGPRINT(errmsg.str());
    }
#endif
}

template <typename T>
MutexGuard<T> Mutex<T>::lock()
{
#ifdef DEBUG_MUTEX
    pthread_mutex_lock(&this->locked_already_mutex);
    if (locked_already == gettid()) {
        std::stringstream me;
        me << ("!!!WARNING!!! Double lock by ") << gettid();
        DBGPRINT(me.str())
    }
    pthread_mutex_unlock(&this->locked_already_mutex);
#endif
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
template <typename T>
template <typename... Args>
Mutex<T>
Mutex<T>::make(Args&&... args)
{
    return Mutex<T>(T(std::forward<Args>(args)...));
}

template <typename T>
Mutex<T>::Mutex(Mutex&& other)
    : inner_mutex(other.inner_mutex)
    , inner_type(std::move(other.inner_type))
{
    // pthread_mutex_t does not have "move support"
    // so we need to move it out manually,
    // so the Mutex dtor doesn't destroy it

    utils::bzero(other.inner_mutex);
}

#ifdef DEBUG_MUTEX
template <typename T>
MutexGuard<T> Mutex<T>::dbglock(const char* fin, int _line, const char* fun)
{
    pthread_mutex_lock(&this->locked_already_mutex);
    if (locked_already == gettid()) {
        std::stringstream me;
        me << ("!Double lock by ") << gettid() << " " << fin << ":" << _line << " firstman: "
           << filename << ":" << line;
        DBGPRINT(me.str());
    }
    pthread_mutex_unlock(&this->locked_already_mutex);
    return MutexGuard<T>(*this, fin, _line, fun);
}
#endif

}