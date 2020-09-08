//
// Created by boat on 7/2/20.
//

#include "futures.hpp"
#include <iostream>

namespace futures {

template <typename T>
auto PollResult<T>::pending() -> PollResult<T>
{
    return PollResult<T>(PollResult::Pending);
}

template <typename T>
auto PollResult<T>::ready(T&& result) -> PollResult<T>
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
auto PollResult<T>::is_ready() const -> bool
{
    return _status == Ready;
}

template <typename T>
auto PollResult<T>::is_pending() const -> bool
{
    return _status == Pending;
}

template <typename T>
auto PollResult<T>::get() -> T
{
    return _result;
}
}
