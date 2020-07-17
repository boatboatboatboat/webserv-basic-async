//
// Created by boat on 7/3/20.
//

#include <iostream>
#include "Waker.hpp"

namespace futures {
	void Waker::operator()() {
		std::cout << "Wake" << std::endl;
	}

    IFuture<void> &Waker::get_future() {
        return *fut;
    }

    Waker::Waker(BoxPtr<IFuture<void>>&& future) {
        fut = std::move(future);
	}
}