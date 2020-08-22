//
// Created by boat on 7/2/20.
//

#ifndef WEBSERV_POLLRESULT_HPP
#define WEBSERV_POLLRESULT_HPP

namespace futures {

template <typename T>
class PollResult {
public:
    PollResult() = delete;
    static PollResult<T> pending();
    static PollResult<T> ready(T&& result);
    bool is_ready() const;
    bool is_pending() const;
    T get();

private:
    enum Status { Pending,
        Ready };
    PollResult(Status status, T&& inner);
    explicit PollResult(Status status);
    Status _status;
    T _result;
};

template <>
class PollResult<void> {
public:
    PollResult() = delete;
    static PollResult<void> pending();
    static PollResult<void> ready();
    bool is_ready() const;
    bool is_pending() const;

private:
    enum Status { Pending,
        Ready };
    explicit PollResult(Status status);
    Status _status;
};
} // namespace futures

#include "PollResult.ipp"

#endif // WEBSERV_POLLRESULT_HPP
