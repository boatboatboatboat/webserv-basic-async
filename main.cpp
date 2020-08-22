#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "futures/ForEachFuture.hpp"
#include "http/HttpParser.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpServer.hpp"
#include "ioruntime/FdLineStream.hpp"
#include "ioruntime/SocketAddr.hpp"
#include "ioruntime/TcpListener.hpp"
#include "http/StringBody.hpp"
#include "utils/utils.hpp"

using futures::FdLineStream;
using futures::ForEachFuture;
using futures::IFuture;
using futures::PollResult;
using http::HttpRequest;
using http::HttpResponse;
using http::HttpResponseBuilder;
using http::HttpServer;
using http::StringBody;
using ioruntime::GlobalIoEventHandler;
using ioruntime::GlobalRuntime;
using ioruntime::IoEventHandler;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;
using ioruntime::TcpListener;

int core()
{
    auto runtime = RuntimeBuilder()
                       .with_workers(8)
                       //.without_workers()
                       .build();

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
                (void)str;
                DBGPRINT("stdin line: " << str);
            }));
    auto request_handler = [](http::HttpRequest& req) {
        DBGPRINT(req.getMethod() << " " << req.getPath());
        std::stringstream out;

        out << "<h1>path: " << req.getPath() << "</h1>";

        auto response = HttpResponseBuilder()
                            .status(HttpResponse::HTTP_STATUS_OK, "OK")
                            .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
                            .body(BoxPtr<StringBody>::make(out.str()))
                            .build();
        return response;
    };
    GlobalRuntime::spawn(HttpServer<decltype(request_handler)>(6969));
    GlobalRuntime::spawn(HttpServer<decltype(request_handler)>(1234));
    (void)request_handler;
    runtime.naive_run();
    return 0;
}

int main()
{
    return (core());
}