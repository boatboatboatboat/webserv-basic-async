//
// Created by boat on 9/29/20.
//

#ifndef WEBSERV_FUNC_SETVALUEFUNCTOR_HPP
#define WEBSERV_FUNC_SETVALUEFUNCTOR_HPP

#include "../boxed/RcPtr.hpp"
#include "../mutex/mutex.hpp"
#include "Functor.hpp"

using boxed::RcPtr;
using mutex::Mutex;

template <typename T>
class SetValueFunctor : public Functor {
public:
    explicit SetValueFunctor(RcPtr<Mutex<T>>&& source, T&& value);
    ~SetValueFunctor() override = default;
    void operator()() override;

private:
    RcPtr<Mutex<T>> _value_mutex;
    T _value;
};

#include "SetValueFunctor.ipp"

#endif //WEBSERV_FUNC_SETVALUEFUNCTOR_HPP
