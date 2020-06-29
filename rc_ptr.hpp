//
// Created by boat on 6/23/20.
//

#ifndef ASYNCTEST2_RC_PTR_HPP
#define ASYNCTEST2_RC_PTR_HPP

#include "mutex.hpp"

template<typename T>
class RcPtr {
public:
	RcPtr() {
		this->control = new RcPtrControlBlock(Mutex<unsigned long>(1));
		this->control->inner = T();
	}

	explicit RcPtr(T other) {
		this->control = new RcPtrControlBlock(Mutex<unsigned long>(1));
		this->control->inner = other;
	}

	RcPtr(const RcPtr<T>& other) {
		auto& refs = other.control->refs.lock().get();

		refs += 1;
		this->control = other.control;
	}

	RcPtr& operator=(const RcPtr<T>& other) {
		auto& refs = other.control->refs.lock().get();

		refs += 1;
		this->control = other.control;
		return *this;
	}

	T& operator*() {
		return control->inner;
	}

	T* operator->() {
		return &control->inner;
	}

	T* get() {
		return &control->inner;
	}

	~RcPtr() {
		auto& refs = this->control->refs.lock().get();

		refs -= 1;
		if (refs == 0)
			delete control;
	}
private:
	class RcPtrControlBlock {
	public:
		explicit RcPtrControlBlock(Mutex<unsigned long> mutex): refs(mutex) {}
		T inner;
		Mutex<unsigned long> refs;
	};
	RcPtrControlBlock* control;
};

#endif //ASYNCTEST2_RC_PTR_HPP
