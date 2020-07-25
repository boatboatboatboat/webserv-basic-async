//
// Created by boat on 7/3/20.
//

#include "Waker.hpp"
#include "../ioruntime/ioruntime.hpp"
#include <iostream>

using ioruntime::GlobalRuntime;

namespace futures {
void Waker::operator()()
{
    // TODO: waker should unregister itself?
    GlobalRuntime::spawn(std::move(fut));
}

IFuture<void>& Waker::get_future() { return *fut; }

Waker::Waker(BoxPtr<IFuture<void>>&& future) { fut = std::move(future); }
} // namespace futures