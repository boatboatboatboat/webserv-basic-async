//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_WAKER_HPP
#define WEBSERV_WAKER_HPP

#include "futures.hpp"
#include "../boxed/BoxPtr.hpp"
#include "../func/Functor.hpp"

using futures::IFuture;
using boxed::BoxPtr;

namespace futures {
	// Forward declarations
	//template<typename T> class IFuture;

	// Classes
	class Waker: public Functor {
	public:
	    Waker(BoxPtr<IFuture<void>>&& future);
	    void operator()() override;
	    IFuture<void>& get_future();
	private:
		BoxPtr<IFuture<void>> fut;
	};
}

#endif //WEBSERV_WAKER_HPP
