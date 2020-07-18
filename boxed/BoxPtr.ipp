//
// Created by boat on 7/3/20.
//

#include "BoxPtr.hpp"

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
BoxPtr<T>::BoxPtr(BoxPtr::t_ptr inner)
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
BoxPtr<T> BoxPtr<T>::operator=(const U& other)
{
    return BoxPtr<T>();
}
}