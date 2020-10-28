//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_FUNC_SETREADYFUNCTOR_HPP
#define WEBSERV_FUNC_SETREADYFUNCTOR_HPP

#include "../boxed/RcPtr.hpp"
#include "../mutex/mutex.hpp"
#include "Functor.hpp"

using boxed::RcPtr;
using mutex::Mutex;

class SetReadyFunctor : public Functor {
public:
    explicit SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source);
    explicit SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source, int meta_fd);
    SetReadyFunctor(SetReadyFunctor&&) noexcept;
    auto operator=(SetReadyFunctor&&) noexcept -> SetReadyFunctor&;
    SetReadyFunctor(SetReadyFunctor const&) = delete;
    auto operator=(SetReadyFunctor const&) -> SetReadyFunctor& = delete;
    ~SetReadyFunctor() override;
    void operator()() override;
    auto dbg_get_mutex() -> RcPtr<Mutex<bool>>&;

private:
    RcPtr<Mutex<bool>> ready_mutex;
    int _meta_fd = -1;
};

#endif //WEBSERV_FUNC_SETREADYFUNCTOR_HPP
