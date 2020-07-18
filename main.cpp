#include <iostream>
#include "ioruntime/ioruntime.hpp"

using ioruntime::GlobalRuntime;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;
using futures::IFuture;
using futures::PollResult;

class FileDescriptor {
public:
	[[nodiscard]] bool has_data_to_read() const {
		return can_read;
	}
	[[nodiscard]] bool has_data_to_write() const {
		return can_write;
	}
private:
	bool can_read;
	bool can_write;
};

class GetLine: public IFuture<void> {
public:
	PollResult<void> poll(Waker&& waker) override {

		if (can_read) {

		} else {
			return PollResult<void>::pending();
		}
	}
private:
	bool can_read = false;
};

int main() {
	auto runtime = RuntimeBuilder()
		.without_workers()
		.build();

	GlobalRuntime::set_runtime(&runtime);
	GlobalRuntime::get().get()->spawn();
	return 0;
}
