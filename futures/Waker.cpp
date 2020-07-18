//
// Created by boat on 7/3/20.
//

#include "Waker.hpp"
#include <iostream>

namespace futures {
void Waker::operator()() { std::cout << "Wake" << std::endl; }

IFuture<void>& Waker::get_future() { return *fut; }

Waker::Waker(BoxPtr<IFuture<void>>&& future) { fut = std::move(future); }
} // namespace futures