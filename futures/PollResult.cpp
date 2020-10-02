//
// Created by boat on 7/3/20.
//

#include "PollResult.hpp"

namespace futures {
// Specialization: void
auto PollResult<void>::ready() -> PollResult<void> { return PollResult<void>(Ready); }

auto PollResult<void>::pending() -> PollResult<void>
{
    return PollResult<void>(Pending);
}

auto PollResult<void>::is_ready() const -> bool { return _status == Ready; }

auto PollResult<void>::is_pending() const -> bool { return _status == Pending; }

PollResult<void>::PollResult(Status status)
    : _status(status)
{
}

} // namespace futures
