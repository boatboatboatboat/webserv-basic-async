//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_RCPTR_HPP
#define WEBSERV_RCPTR_HPP

#include "../mutex/mutex.hpp"

using mutex::Mutex;

template <class T, class U>
concept Derived = std::is_base_of<U, T>::value;

namespace boxed {
template <typename T>
class RcPtr {
public:
    typedef std::remove_extent<T> element_type;

    RcPtr()
    {
        this->refs_mutex = new Mutex<unsigned long>(1);
        this->inner = new (std::nothrow) T();
        if (this->inner == nullptr) {
            delete this->refs_mutex;
            throw std::bad_alloc();
        }
    }

    explicit RcPtr(T&& other)
    {
        this->refs_mutex = new Mutex<unsigned long>(1);
        this->inner = new (std::nothrow) T(std::move(other));
        if (this->inner == nullptr) {
            delete this->refs_mutex;
            throw std::bad_alloc();
        }
    }

    template <typename... Args>
    static RcPtr<T> make(Args&&... args)
    {
        return RcPtr<T>(std::move(T(std::forward<Args>(args)...)));
    }

    static RcPtr<T> uninitialized()
    {
        return RcPtr<T>(0, 0);
    }

    template<Derived<T> U>
    RcPtr(RcPtr<U>& other)
    {
        {
            auto refs = other.get_raw_mutex()->lock();
            *refs += 1;
        }
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.inner;
    }

    template<Derived<T> U>
    RcPtr& operator=(RcPtr<U>& other)
    {
        {
            auto refs = other.get_raw_mutex()->lock();
            *refs += 1;
        }
        this->refs_mutex = other.get_raw_mutex();
        this->control = other.control;
        return *this;
    }

    template<Derived<T> U>
    RcPtr& operator=(RcPtr<U>&& other)
    {
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.inner;
        other.leak();
        return *this;
    }

    template<Derived<T> U>
    RcPtr(RcPtr<U>&& other)
    {
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.get();
        other.leak();
    }

    T& operator*() { return *this->inner; }

    const T& operator*() const { return *this->inner; }

    T* operator->() { return this->inner; }

    const T* operator->() const { return this->inner; }

    T* get() { return this->inner; }

    const T* get() const { return this->inner; }

    void leak() { this->inner = nullptr; this->refs_mutex = nullptr; }

    Mutex<unsigned long>* get_raw_mutex() { return this->refs_mutex; }

    ~RcPtr()
    {
        // if the RcPtr is moved, refs_mutex will be null
        if (this->refs_mutex == nullptr)
            return;

        {
            auto refs = this->refs_mutex->lock();

            *refs -= 1;
            if (*refs == 0) {
                delete this->inner;
            } else {
                return;
            }
        }
        // we delete it after the guard has been dropped
        // otherwise the guard will try to change a stale mutex
        delete refs_mutex;
    }

private:
    explicit RcPtr(int a, int b);
    T* inner;
    Mutex<unsigned long>* refs_mutex;
};

template <typename T>
RcPtr<T>::RcPtr(int a, int b)
{
    (void)a;
    (void)b;
    this->inner = nullptr;
    this->refs_mutex = nullptr;
}

template<typename T, size_t N>
class RcPtr<T[N]> {
public:
    typedef std::remove_extent<T> element_type;

    RcPtr()
    {
        this->refs_mutex = new Mutex<unsigned long>(1);
        this->inner = new (std::nothrow) T();
        if (this->inner == nullptr) {
            delete this->refs_mutex;
            throw std::bad_alloc();
        }
    }

    explicit RcPtr(T&& other)
    {
        this->refs_mutex = new Mutex<unsigned long>(1);
        this->inner = new (std::nothrow) T(std::move(other));
        if (this->inner == nullptr) {
            delete this->refs_mutex;
            throw std::bad_alloc();
        }
    }

    template <typename... Args>
    static RcPtr<T> make(Args&&... args)
    {
        return RcPtr<T>(std::move(T(std::forward<Args>(args)...)));
    }

    static RcPtr<T> uninitialized()
    {
        return RcPtr<T>(0, 0);
    }

    template<Derived<T> U>
    RcPtr(RcPtr<U>& other)
    {
        {
            auto refs = other.get_raw_mutex()->lock();
            *refs += 1;
        }
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.inner;
    }

    template<Derived<T> U>
    RcPtr& operator=(RcPtr<U>& other)
    {
        {
            auto refs = other.get_raw_mutex()->lock();
            *refs += 1;
        }
        this->refs_mutex = other.get_raw_mutex();
        this->control = other.control;
        return *this;
    }

    template<Derived<T> U>
    RcPtr& operator=(RcPtr<U>&& other)
    {
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.inner;
        other.leak();
        return *this;
    }

    template<Derived<T> U>
    RcPtr(RcPtr<U>&& other)
    {
        this->refs_mutex = other.get_raw_mutex();
        this->inner = other.get();
        other.leak();
    }

    T& operator*() { return *this->inner; }

    const T& operator*() const { return *this->inner; }

    T* operator->() { return this->inner; }

    const T* operator->() const { return this->inner; }

    T* get() { return this->inner; }

    const T* get() const { return this->inner; }

    void leak() { this->inner = nullptr; this->refs_mutex = nullptr; }

    Mutex<unsigned long>* get_raw_mutex() { return this->refs_mutex; }

    ~RcPtr()
    {
        // if the RcPtr is moved, refs_mutex will be null
        if (this->refs_mutex == nullptr)
            return;

        {
            auto refs = this->refs_mutex->lock();

            *refs -= 1;
            if (*refs == 0) {
                delete[] this->inner;
            } else {
                return;
            }
        }
        // we delete it after the guard has been dropped
        // otherwise the guard will try to change a stale mutex
        delete refs_mutex;
    }

private:
    explicit RcPtr(int a, int b);
    T* inner;
    Mutex<unsigned long>* refs_mutex;
};

template<typename T, typename U>
bool operator==(RcPtr<T> const& lhs, RcPtr<U> const& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool operator!=(RcPtr<T> const& lhs, RcPtr<U> const& rhs) noexcept {
    return !(lhs == rhs);
}

} // namespace boxed


#endif // WEBSERV_RCPTR_HPP
