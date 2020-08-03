//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_ISTREAM_HPP
#define WEBSERV_FUTURES_ISTREAM_HPP

#include "futures.hpp"

namespace futures {
// Forward declarations

class Waker;
template <typename T>
class StreamPollResult;

// Classes
template <typename T>
class IStream {
public:
    virtual ~IStream() = 0;
    virtual StreamPollResult<T> poll_next(Waker&& waker) = 0;
};
}

#endif //WEBSERV_FUTURES_ISTREAM_HPP
