#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "futures/ForEachFuture.hpp"
#include "http/HttpParser.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpServer.hpp"
#include "http/InfiniteBody.hpp"
#include "http/StringBody.hpp"
#include "ioruntime/FdLineStream.hpp"
#include "ioruntime/SocketAddr.hpp"
#include "ioruntime/TcpListener.hpp"
#include "utils/utils.hpp"
#include <signal.h>

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
    // ignore SIGPIPE,
    // a slowloris attack can cause so many SIGPIPEs (to the point of 30k queued),
    // that all threads are preoccupied with running the signal handler
    signal(SIGPIPE, SIG_IGN);
    try {
        auto runtime = RuntimeBuilder()
                           .with_workers(4)
                           // .without_workers()
                           .build();
        auto io_event = BoxPtr<IoEventHandler>::make();
        runtime.register_io_handler(std::move(io_event));
        GlobalRuntime::set_runtime(&runtime);
        GlobalRuntime::spawn(
            FdLineStream(STDIN_FILENO)
                .for_each<FdLineStream>([](std::string& str) {
                    (void)str;
                    INFOPRINT("stdin line: " << str);
                }));
        GlobalRuntime::spawn(HttpServer(1234, [](http::HttpRequest& req) {
            (void)req;
            INFOPRINT("Handle request");
            auto response = HttpResponseBuilder()
                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
                .build();
            return response;
        }));
        GlobalRuntime::spawn(HttpServer(1235, [](http::HttpRequest& req) {
            std::stringstream out;

            out << "<h1>path: " << req.getPath() << "</h1>" << std::endl;
            auto response = HttpResponseBuilder()
                .status(HttpResponse::HTTP_STATUS_OK)
                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
                .body(BoxPtr<StringBody>::make(out.str()))
                .build();
            return response;
        }));
        GlobalRuntime::spawn(HttpServer(1236, [](http::HttpRequest& req) {
            (void)req;
            auto response = HttpResponseBuilder()
                .status(HttpResponse::HTTP_STATUS_OK)
                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
                .body(BoxPtr<FileDescriptor>::make(open("test2.txt", O_RDONLY)))
                .build();
            return response;
        }));
        GlobalRuntime::spawn(HttpServer(1237, [](http::HttpRequest& req) {
            (void)req;
            throw std::runtime_error("Handler error test");
            return HttpResponseBuilder().build();
        }));
        GlobalRuntime::spawn(HttpServer(1238, [](http::HttpRequest& req) {
            (void)req;
            auto response = HttpResponseBuilder()
                .status(HttpResponse::HTTP_STATUS_OK)
                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
                .body(BoxPtr<InfiniteBody>::make())
                .build();
            return response;
        }));
        runtime.naive_run();
    } catch (std::exception& e) {
        std::cerr << "Failed to load runtime: " << e.what() << std::endl;
    }
    return 0;
}

int main()
{
    return (core());
}