//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_IFUTURE_HPP
#define WEBSERV_IFUTURE_HPP

#include "futures.hpp"

namespace futures {
	// Forward declarations
	class Waker;

	// Classes
	template<typename T = void>
	class IFuture {
	public:
		virtual PollResult<T> poll(Waker&& waker) = 0;
	};
}

#endif //WEBSERV_IFUTURE_HPP
