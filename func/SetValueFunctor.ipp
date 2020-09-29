//
// Created by boat on 9/29/20.
//

#include "SetValueFunctor.hpp"

template <typename T>
SetValueFunctor<T>::SetValueFunctor(RcPtr<Mutex<T>>&& source, T&& value):
    _value_mutex(std::move(source)),
    _value(std::move(value))
{
}

template <typename T>
void SetValueFunctor<T>::operator()()
{
    auto lock = (*_value_mutex).lock();
    *lock = std::move(_value);
}
