//
// Created by boat on 7/3/20.
//

#include "BoxPtr.hpp"
#include <iostream>

namespace boxed {
template <typename T>
BoxPtr<T>::BoxPtr()
    : inner(t_ptr())
{
}

template <typename T>
BoxPtr<T>::~BoxPtr() { delete inner; }

template <typename T>
T& BoxPtr<T>::operator*()
{
    return *inner;
}

template <typename T>
T* BoxPtr<T>::operator->()
{
    return inner;
}

template <typename T>
T* BoxPtr<T>::get()
{
    return inner;
}

template <typename T>
BoxPtr<T>::BoxPtr(BoxPtr::t_ptr inner): inner(inner)
{
}

template <typename T>
template <typename... Args>
BoxPtr<T> BoxPtr<T>::make(Args&&... args)
{
    return BoxPtr<T>(new T(std::forward<Args>(args)...));
}


template <typename T>
template <typename U>
BoxPtr<T>& BoxPtr<T>::operator=(BoxPtr<U>&& other)
{
    if (static_cast<const void *>(other.get()) == static_cast<const void *>(inner))
        return *this;
    inner = other.get();
    other.leak();
    return *this;
}

template <typename T>
template <typename U>
BoxPtr<T>::BoxPtr(BoxPtr<U>&& other) {
    inner = other.get();
    other.leak();
}

    template<typename T>
    const T *BoxPtr<T>::get() const {
        return this;
    }

    template<typename T>
    void BoxPtr<T>::leak() {
        inner = nullptr;
    }
}