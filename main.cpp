#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "fs/File.hpp"
#include "futures/ForEachFuture.hpp"
#include "futures/SelectFuture.hpp"
#include "http/HttpParser.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpServer.hpp"
#include "http/InfiniteBody.hpp"
#include "http/StringBody.hpp"
#include "ioruntime/FdLineStream.hpp"
#include "ioruntime/FdStringReadFuture.hpp"
#include "ioruntime/TimeoutEventHandler.hpp"
#include "ioruntime/TimeoutFuture.hpp"
#include "net/SocketAddr.hpp"
#include "net/TcpListener.hpp"
#include "utils/utils.hpp"
#include "json/Json.hpp"
#include <csignal>

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
using ioruntime::TimeoutEventHandler;
using ioruntime::TimeoutEventHandler;
using ioruntime::TimeoutFuture;

int core()
{
    // ignore SIGPIPE,
    // a dos attack can cause so many SIGPIPEs (to the point of 30k queued),
    // that all threads are preoccupied with running the signal handler
    signal(SIGPIPE, SIG_IGN);
    try {
        json::Json config;
        std::string error;
        std::string abc;
        {
            auto rt = RuntimeBuilder()
                .without_workers()
                .build();

            rt.register_handler(BoxPtr<IoEventHandler>::make(), Runtime::HandlerType::Io);
            rt.globalize();

            GlobalRuntime::spawn(ioruntime::FdStringReadFuture(fs::File::open("config.json"), abc));
            rt.naive_run();

            INFOPRINT("PARSE STATR");

            config = json::Json::parse(abc, error);
            INFOPRINT("PARSE SENDER");
        }
        if (!error.empty()) {
            // eh
            throw std::runtime_error(error);
        }
//
//        auto runtime = RuntimeBuilder()
//                           .with_workers(4)
//                           .build();
//        runtime.register_handler(BoxPtr<IoEventHandler>::make(), Runtime::HandlerType::Io);
//        runtime.register_handler(BoxPtr<TimeoutEventHandler>::make(), Runtime::HandlerType::Timeout);
//        runtime.globalize();
//        GlobalRuntime::spawn(
//            FdLineStream(STDIN_FILENO)
//                .for_each<FdLineStream>([](std::string& str) {
//                    (void)str;
//                    INFOPRINT("stdin line: " << str);
//                }));
//        GlobalRuntime::spawn(HttpServer(1234, [](http::HttpRequest& req) {
//            (void)req;
//            INFOPRINT("Handle request");
//            auto response = HttpResponseBuilder()
//                                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
//                                .build();
//            return response;
//        }));
//
//        GlobalRuntime::spawn(HttpServer(1235, [](http::HttpRequest& req) {
//            std::stringstream out;
//
//            out << "<h1>path: " << req.getPath() << "</h1><br>"
//                << "<a>method: " << req.getMethod() << "</a><br>"
//                << "<a>version: " << req.getVersion() << "</a><hr>"
//                << "<h1>Queries:</h1><br><table><tr><th>key</th><th>value</th></tr>";
//            for (auto& query : req.getQuery()) {
//                out << "<tr><td>" << query.first << "</td><td>" << query.second << "</td></tr>";
//            }
//            out << "</table><hr><h1>Headers:</h1><table><tr><th>key</th><th>value</th></tr>";
//            for (auto& header : req.getHeaders()) {
//                out << "<tr><td>" << header.first << "</td><td>" << header.second << "</td></tr>";
//            }
//            out << "</table><hr><h1>Body:</h1><br>" << req.getBody();
//
//            auto response = HttpResponseBuilder()
//                                .status(http::HTTP_STATUS_OK)
//                                .header(http::header::CONTENT_TYPE, "text/html; charset=utf-8")
//                                .body(BoxPtr<StringBody>::make(out.str()))
//                                .build();
//            return response;
//        }));
//
//        GlobalRuntime::spawn(HttpServer(1236, [](http::HttpRequest& req) {
//            (void)req;
//            auto response = HttpResponseBuilder()
//                                .status(http::HTTP_STATUS_OK)
//                                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
//                                .body(BoxPtr<FileDescriptor>::make(open("test2.txt", O_RDONLY)))
//                                .build();
//            return response;
//        }));
//
//        GlobalRuntime::spawn(HttpServer(1237, [](http::HttpRequest& req) {
//            (void)req;
//            throw std::runtime_error("Handler error test");
//            return HttpResponseBuilder().build();
//        }));
//
//        GlobalRuntime::spawn(HttpServer(1238, [](http::HttpRequest& req) {
//            (void)req;
//            auto response = HttpResponseBuilder()
//                                .status(http::HTTP_STATUS_OK)
//                                .header(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html; charset=utf-8")
//                                .body(BoxPtr<InfiniteBody>::make())
//                                .build();
//            return response;
//        }));
//        runtime.naive_run();
    } catch (std::exception& e) {
        std::cerr << "Failed to load runtime: " << e.what() << std::endl;
    }
    return 0;
}

int main()
{
    return (core());
}