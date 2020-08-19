#include <iostream>
#include <fcntl.h>
#include "ioruntime/ioruntime.hpp"

#include "futures/FdLineStream.hpp"
#include "futures/ForEachFuture.hpp"
#include "util/util.hpp"

using ioruntime::GlobalRuntime;
using ioruntime::GlobalIoEventHandler;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;
using futures::IFuture;
using futures::PollResult;
using futures::ForEachFuture;
using futures::FdLineStream;

using ioruntime::IoEventHandler;

int core() {
	auto runtime = RuntimeBuilder()
		.with_workers(6)
		//.without_workers()
		.build();

	auto file = open("test.txt", O_RDONLY);
	auto file2 = open("test2.txt", O_RDONLY);
	auto io_event = BoxPtr<IoEventHandler>::make();
	runtime.register_io_handler(std::move(io_event));
	GlobalRuntime::set_runtime(&runtime);
	GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
	        std::move(FdLineStream(STDIN_FILENO)), [](std::string& x)
	        {
	            std::string y("in line: ");
	            y.append(x);
	            DBGPRINT(y);
	        }));
    GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
            std::move(FdLineStream(file)), [](std::string& x)
            {
                std::string y("file line: ");
                y.append(x);
                DBGPRINT(y);
            }));
    GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
            std::move(FdLineStream(file2)), [](std::string& x)
            {
                std::string y("file2 line: ");
                y.append(x);
                DBGPRINT(y);
            }));
	runtime.naive_run();
	return 0;
}

int main() {
    return (core());
}