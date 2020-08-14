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

    RcPtr(RcPtr<T>& other)
    {
		{
			auto &refs = other.refs_mutex->lock().get();
			refs += 1;
		}
        this->refs_mutex = other.refs_mutex;
        this->inner = other.inner;
    }

    RcPtr& operator=(RcPtr<T>& other)
    {
		{
			auto &refs = other.refs_mutex->lock().get();
			refs += 1;
		}
		this->refs_mutex = other.refs_mutex;
        this->control = other.control;
        return *this;
    }

    RcPtr& operator=(RcPtr<T>&& other)
    {
		{
			auto &refs = other.refs_mutex->lock().get();
			refs += 1;
		}
		this->refs_mutex = other.refs_mutex;
        this->inner = other.inner;
        other.inner = nullptr;
        other.refs_mutex = nullptr;
        return *this;
    }

    RcPtr(RcPtr<T>&& other)
    {
		{
			auto &refs = other.refs_mutex->lock().get();
			refs += 1;
		}
		this->refs_mutex = other.refs_mutex;
        this->inner = other.inner;
        other.inner = nullptr;
        other.refs_mutex = nullptr;
    }

    T& operator*() { return *this->inner; }

    T* operator->() { return this->inner; }

    T* get() { return this->inner; }

    ~RcPtr()
    {
		{
			auto &refs = this->refs_mutex->lock().get();

			refs -= 1;
			if (refs == 0)
				delete this->inner;
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
} // namespace boxed

#endif // WEBSERV_RCPTR_HPP
