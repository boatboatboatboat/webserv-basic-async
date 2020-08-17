//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STREAMPOLLRESULT_HPP
#define WEBSERV_FUTURES_STREAMPOLLRESULT_HPP

namespace futures {
template <typename T>
class StreamPollResult {
public:
    enum Status { Pending,
        Ready,
        Finished };
    StreamPollResult() = delete;
    static StreamPollResult<T> pending();
    static StreamPollResult<T> ready(T&& result);
    static StreamPollResult<T> finished();
    [[nodiscard]] Status get_status() const;
    T& get();

private:
    StreamPollResult(Status status, T inner);
    explicit StreamPollResult(Status status);
    Status _status;
    T _result;
};
}

#include "StreamPollResult.ipp"

#endif //WEBSERV_FUTURES_STREAMPOLLRESULT_HPP
