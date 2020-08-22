//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STREAMPOLLRESULT_IPP
#define WEBSERV_FUTURES_STREAMPOLLRESULT_IPP

#include "StreamPollResult.hpp"
#include <algorithm>

namespace futures {
template <typename T>
StreamPollResult<T>::StreamPollResult(StreamPollResult::Status status, T&& inner)
    : _status(status)
    , _result(std::move(inner))
{
}

template <typename T>
StreamPollResult<T>::StreamPollResult(StreamPollResult::Status status)
    : _status(status)
{
}

template <typename T>
StreamPollResult<T>
StreamPollResult<T>::pending()
{
    return StreamPollResult<T>(StreamPollResult::Pending);
}
template <typename T>
StreamPollResult<T>
StreamPollResult<T>::ready(T&& result)
{
    return StreamPollResult<T>(StreamPollResult::Ready, std::move(result));
}

template <typename T>
T& StreamPollResult<T>::get()
{
    return _result;
}
template <typename T>
StreamPollResult<T>
StreamPollResult<T>::finished()
{
    return StreamPollResult<T>(StreamPollResult::Finished);
}

template <typename T>
typename StreamPollResult<T>::Status
StreamPollResult<T>::get_status() const
{
    return _status;
}
template <typename T>
StreamPollResult<T> StreamPollResult<T>::pending(T&& uninitialized)
{
    return StreamPollResult<T>(Pending, std::move(uninitialized));
}

template <typename T>
StreamPollResult<T> StreamPollResult<T>::finished(T&& uninitialized)
{
    return StreamPollResult<T>(Finished, std::move(uninitialized));
}

}

#endif //WEBSERV_FUTURES_STREAMPOLLRESULT_IPP
