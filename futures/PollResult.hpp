//
// Created by boat on 7/2/20.
//

#ifndef WEBSERV_POLLRESULT_HPP
#define WEBSERV_POLLRESULT_HPP

#include "../option/optional.hpp"

namespace futures {

using option::optional;

template <typename T>
class PollResult {
public:
    PollResult() = delete;
    [[nodiscard]] static auto pending() -> PollResult<T>;
    [[nodiscard]] static auto ready(T&& result) -> PollResult<T>;
    [[nodiscard]] auto is_ready() const -> bool;
    [[nodiscard]] auto is_pending() const -> bool;
    auto get() -> T;

private:
    enum Status { Pending,
        Ready };
    PollResult(Status status, T&& inner);
    explicit PollResult(Status status);
    Status _status;
    optional<T> _result;
};

template <>
class PollResult<void> {
public:
    PollResult() = delete;
    [[nodiscard]] static auto pending() -> PollResult<void>;
    [[nodiscard]] static auto ready() -> PollResult<void>;
    [[nodiscard]] auto is_ready() const -> bool;
    [[nodiscard]] auto is_pending() const -> bool;

private:
    enum Status { Pending,
        Ready };
    explicit PollResult(Status status);
    Status _status;
};
} // namespace futures

#include "PollResult.ipp"

#endif // WEBSERV_POLLRESULT_HPP
