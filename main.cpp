#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>

#include "futures/FdLineStream.hpp"
#include "futures/ForEachFuture.hpp"
#include "util/util.hpp"

using futures::FdLineStream;
using futures::ForEachFuture;
using futures::IFuture;
using futures::PollResult;
using ioruntime::GlobalIoEventHandler;
using ioruntime::GlobalRuntime;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;

using ioruntime::IoEventHandler;

int core()
{
    auto runtime = RuntimeBuilder()
                       .with_workers(6)
                       //.without_workers()
                       .build();

    auto file = open("test.txt", O_RDONLY);
    auto file2 = open("test2.txt", O_RDONLY);
    auto io_event = BoxPtr<IoEventHandler>::make();
    runtime.register_io_handler(std::move(io_event));
    GlobalRuntime::set_runtime(&runtime);
    GlobalRuntime::spawn(ForEachFuture<FdLineStream, std::string>(
        FdLineStream(STDIN_FILENO), [](std::string& x) {
            std::string y("stdin line: ");
            y.append(x);
            DBGPRINT(y);
        }));
    GlobalRuntime::spawn(ForEachFuture<FdLineStream, std::string>(
        FdLineStream(file), [](std::string& x) {
            std::string y("file line: ");
            y.append(x);
            DBGPRINT(y);
        }));
    GlobalRuntime::spawn(ForEachFuture<FdLineStream, std::string>(
        FdLineStream(file2), [](std::string& x) {
            std::string y("file2 line: ");
            y.append(x);
            DBGPRINT(y);
        }));
    runtime.naive_run();
    return 0;
}

int main()
{
    return (core());
}