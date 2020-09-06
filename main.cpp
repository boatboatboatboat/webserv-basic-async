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
using ioruntime::TimeoutFuture;

void build_from_config(json::Json const& config, RuntimeBuilder& builder) {
    // Load workers
    if (config["workers"].is_number()) {
        auto worker_count = static_cast<int64_t>(config["workers"].number_value());

        if (worker_count < 0) {
            throw std::runtime_error("Worker count is negative");
        } else if (worker_count == 0) {
            INFOPRINT("Runtime will use no worker threads");
            builder = builder.without_workers();
        } else {
            INFOPRINT("Runtime will use " << worker_count << " worker thread(s)");
            builder = builder.with_workers(worker_count);
        }
    } else {
        throw std::runtime_error("Worker count is not a number");
    }
}

void spawn_from_config(json::Json const& config) {
    if (!config["http"].is_object())
        throw std::runtime_error("http field is either not set or not an object");

    if (config["http"]["servers"].is_array()) {
        auto servers = config["http"]["servers"].array_items();

        if (servers.empty()) {
            throw std::runtime_error("http.servers has no items -- it is entirely useless");
        }

        bool default_host_set = false, default_port_set = false;
        uint16_t default_port;
        net::IpAddress default_ip = net::IpAddress::v4(0);

        for (auto const& server : servers) {
            if (server.is_object()) {
                auto const& listen = server["listen"];

                if (listen.is_null()) {
                    if (!(default_host_set && default_port_set)) {
                        throw std::runtime_error("no listen field without a default host:port combo set");;
                    } else {
                        uint16_t some_port = default_port;
                        net::IpAddress some_ip = default_ip;
                    }
                } else if (!listen.is_array()) {
                    throw std::runtime_error("listen field is not an array");
                } else {
                    for (auto const& combination: listen.array_items()) {
                        auto ip_field = combination["ip"];
                        auto port_field = combination["port"];
                        if (port_field.is_null()) {
                            if (default_port_set) {

                            }
                        }
                    }
                }
                default_host_set = true;
                default_port_set = true;
                (void)server_info;
            } else {
                throw std::runtime_error("http.servers item is not an object");
            }
        }
    } else {
        throw std::runtime_error("http.servers is either not set or not an array");
    }
}

int main()
{
    // ignore SIGPIPE,
    // a dos attack can cause so many SIGPIPEs (to the point of 30k queued),
    // that all threads are preoccupied with running the signal handler
    signal(SIGPIPE, SIG_IGN);
    try {

        json::Json config;
        std::string error;

        {
            auto rt = RuntimeBuilder()
                .without_workers()
                .build();

            rt.globalize();

            std::string config_str;
            GlobalRuntime::spawn(ioruntime::FdStringReadFuture(fs::File::open("config.json"), config_str));
            rt.naive_run();

            config = json::Json::parse(config_str, error);
        }

        if (!error.empty()) {
            ERRORPRINT("bad config: " << error);
            throw std::runtime_error("Config error");
        }

        auto builder = RuntimeBuilder();

        build_from_config(config, builder);

        auto runtime = builder.build();
        runtime.globalize();

        spawn_from_config(config);

        runtime.naive_run();
    } catch (std::exception& e) {
        ERRORPRINT("Failed to load runtime: " << e.what());
    }
    return 0;
}