//
// Created by boat on 7/3/20.
//

#include "PollResult.hpp"

namespace futures {
	// Specialization: void
	PollResult<void> PollResult<void>::ready() {
		return PollResult<void>(Ready);
	}

	PollResult<void> PollResult<void>::pending() {
		return PollResult<void>(Pending);
	}

	bool PollResult<void>::is_ready() const {
		return _status == Ready;
	}

	bool PollResult<void>::is_pending() const {
		return _status == Pending;
	}

	PollResult<void>::PollResult(Status status):
			_status(status) {}
}
