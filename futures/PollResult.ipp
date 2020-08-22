//
// Created by boat on 7/2/20.
//

#include "futures.hpp"
#include <iostream>

namespace futures {

template <typename T>
PollResult<T> PollResult<T>::pending()
{
    return PollResult<T>(PollResult::Pending);
}

template <typename T>
PollResult<T> PollResult<T>::ready(T&& result)
{
    return PollResult<T>(PollResult::Ready, std::move(result));
}

template <typename T>
PollResult<T>::PollResult(PollResult::Status status, T&& result)
    : _status(status)
    , _result(std::forward<T>(result))

{
}

template <typename T>
PollResult<T>::PollResult(PollResult::Status status)
{
    _status = status;
}

template <typename T>
bool PollResult<T>::is_ready() const
{
    return _status == Ready;
}

template <typename T>
bool PollResult<T>::is_pending() const
{
    return _status == Pending;
}

template <typename T>
T PollResult<T>::get()
{
    return _result;
}
}
