//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_WAKER_HPP
#define WEBSERV_WAKER_HPP

#include "../boxed/BoxPtr.hpp"
#include "../func/Functor.hpp"
#include "futures.hpp"

using boxed::BoxPtr;
using futures::IFuture;

namespace futures {
// Forward declarations
// template<typename T> class IFuture;

// Classes
class Waker : public Functor {
public:
    Waker(BoxPtr<IFuture<void>>&& future);
    void operator()() override;
    IFuture<void>& get_future();

private:
    BoxPtr<IFuture<void>> fut;
};
} // namespace futures

#endif // WEBSERV_WAKER_HPP
