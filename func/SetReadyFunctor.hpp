//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_FUNC_SETREADYFUNCTOR_HPP
#define WEBSERV_FUNC_SETREADYFUNCTOR_HPP

#include "Functor.hpp"
#include "../mutex/mutex.hpp"
#include "../boxed/RcPtr.hpp"

using boxed::RcPtr;
using mutex::Mutex;

class SetReadyFunctor : public Functor {
public:
    explicit SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source);
    ~SetReadyFunctor() override = default;
    void operator()() override;

private:
    RcPtr<Mutex<bool>> ready_mutex;
};

#endif //WEBSERV_FUNC_SETREADYFUNCTOR_HPP
