//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_FUNCTOR_HPP
#define WEBSERV_FUNCTOR_HPP

#include "../boxed/BoxPtr.hpp"

class Functor {
public:
    virtual ~Functor() = 0;
    virtual void operator()() = 0;
};

using BoxFunctor = boxed::BoxPtr<Functor>;

#endif // WEBSERV_FUNCTOR_HPP
