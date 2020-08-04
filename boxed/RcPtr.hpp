//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_RCPTR_HPP
#define WEBSERV_RCPTR_HPP

#include "../mutex/mutex.hpp"

using mutex::Mutex;

namespace boxed {
template <typename T>
class RcPtr {
public:
    RcPtr()
    {
        this->control = new RcPtrControlBlock(Mutex<unsigned long>(1));
        this->control->inner = T();
    }

    explicit RcPtr(T&& other)
    {
        this->control = new RcPtrControlBlock(Mutex<unsigned long>(1));
        this->control->inner = std::move(other);
    }

    template <typename... Args>
    static RcPtr<T> make(Args&&... args)
    {
        return RcPtr<T>(std::move(T(std::forward<Args>(args)...)));
    }

    RcPtr(const RcPtr<T>& other)
    {
        auto& refs = other.control->refs.lock().get();

        refs += 1;
        this->control = other.control;
    }

    RcPtr& operator=(const RcPtr<T>& other)
    {
        auto& refs = other.control->refs.lock().get();

        refs += 1;
        this->control = other.control;
        return *this;
    }

    T& operator*() { return control->inner; }

    T* operator->() { return &control->inner; }

    T* get() { return &control->inner; }

    ~RcPtr()
    {
        auto& refs = this->control->refs.lock().get();

        refs -= 1;
        if (refs == 0)
            delete control;
    }

private:
    class RcPtrControlBlock {
    public:
        explicit RcPtrControlBlock(Mutex<unsigned long> mutex)
            : refs(std::move(mutex))
        {
        }

        T inner;
        Mutex<unsigned long> refs;
    };

    RcPtrControlBlock* control;
};
} // namespace boxed

#endif // WEBSERV_RCPTR_HPP
