//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_IFUTURE_HPP
#define WEBSERV_IFUTURE_HPP

#include "futures.hpp"

namespace futures {
// Forward declarations
class Waker;
template <typename T>
class PollResult;

// Classes
template <typename T>
class IFuture {
public:
    virtual ~IFuture() = 0;
    virtual PollResult<T> poll(Waker&& waker) = 0;
};

template <typename T>
IFuture<T>::~IFuture() { }
} // namespace futures

#endif // WEBSERV_IFUTURE_HPP
