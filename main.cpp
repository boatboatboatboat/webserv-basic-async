#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "futures/FdLineStream.hpp"
#include "futures/ForEachFuture.hpp"
#include "ioruntime/SocketAddr.hpp"
#include "ioruntime/TcpListener.hpp"
#include "utils/utils.hpp"

using futures::FdLineStream;
using futures::ForEachFuture;
using futures::IFuture;
using futures::PollResult;
using ioruntime::GlobalIoEventHandler;
using ioruntime::GlobalRuntime;
using ioruntime::IoEventHandler;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;
using ioruntime::TcpListener;

int core()
{
    auto runtime = RuntimeBuilder()
                       //.with_workers(6)
                       .without_workers()
                       .build();

    auto file = open("test.txt", O_RDONLY);
    auto file2 = open("test2.txt", O_RDONLY);
    auto io_event = BoxPtr<IoEventHandler>::make();
    runtime.register_io_handler(std::move(io_event));
    GlobalRuntime::set_runtime(&runtime);
    //    GlobalRuntime::spawn(
    //        HttpServer(8080)
    //            .serve([](HttpRequest& request) {
    //                auto response = HttpResponseBuilder();
    //                // ... do something with request and response
    //                return response;
    //            }));
    GlobalRuntime::spawn(
        FdLineStream(STDIN_FILENO)
            .for_each<FdLineStream>([](std::string& str) {
                std::stringstream dbgmsg;
                dbgmsg << "stdin line: " << str;
                DBGPRINT(dbgmsg.str());
            }));
    GlobalRuntime::spawn(
        FdLineStream(file)
            .for_each<FdLineStream>([](std::string& str) {
                std::stringstream dbgmsg;
                dbgmsg << "file line: " << str;
                DBGPRINT(dbgmsg.str());
            }));
    GlobalRuntime::spawn(
        FdLineStream(file2)
            .for_each<FdLineStream>([](std::string& str) {
                std::stringstream dbgmsg;
                dbgmsg << "file2 line: " << str;
                DBGPRINT(dbgmsg.str());
            }));
    GlobalRuntime::spawn(
        TcpListener(1234)
            .for_each<TcpListener>([](ioruntime::TcpStream& stream) {
                std::stringstream dbgstr;
                dbgstr
                    << "TCP connection accepted from "
                    << stream.get_addr();
                DBGPRINT(dbgstr.str());
                GlobalRuntime::spawn(
                    std::move(stream).respond([](auto& str) {
                        DBGPRINT(str);
                        return str;
                    }));
            }));
    (void)file;
    (void)file2;
    runtime.naive_run();
    return 0;
}

int main()
{
    return (core());
}