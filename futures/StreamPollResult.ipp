//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STREAMPOLLRESULT_IPP
#define WEBSERV_FUTURES_STREAMPOLLRESULT_IPP

#include "StreamPollResult.hpp"

namespace futures {
template <typename T>
StreamPollResult<T>::StreamPollResult(StreamPollResult::Status status, T inner)
{
    _status = status;
    _result = inner;
}

template <typename T>
StreamPollResult<T>::StreamPollResult(StreamPollResult::Status status)
{
    _status = status;
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
    return StreamPollResult<T>(StreamPollResult::Ready);
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
StreamPollResult::Status
StreamPollResult<T>::get_status() const
{
    return _status;
}

}

#endif //WEBSERV_FUTURES_STREAMPOLLRESULT_IPP
