//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_WAKER_HPP
#define WEBSERV_WAKER_HPP

#include "../ioruntime/IExecutor.hpp"
#include "../boxed/RcPtr.hpp"
#include "../func/Functor.hpp"
#include "futures.hpp"

using boxed::RcPtr;
using futures::IFuture;

namespace futures {
// Forward declarations
// template<typename T> class IFuture;
class Task;

// Classes
class Waker : public Functor {
public:
    Waker() = delete;
    Waker(Waker& waker);
    ~Waker() = default;
    Waker(RcPtr<Task>&& future);
    void operator()() override;
    RcPtr<Task>& get_task();

private:
    RcPtr<Task> task;
};
} // namespace futures

#endif // WEBSERV_WAKER_HPP
