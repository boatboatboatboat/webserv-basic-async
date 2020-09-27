#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "args/Args.hpp"
#include "config/Config.hpp"
#include "fs/File.hpp"
#include "futures/ForEachFuture.hpp"
#include "futures/SelectFuture.hpp"
#include "http/DirectoryBody.hpp"
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
#include "regex/Regex.hpp"
#include "utils/utils.hpp"
#include "json/Json.hpp"
#include <csignal>

using futures::FdLineStream;
using futures::ForEachFuture;
using futures::IFuture;
using futures::PollResult;
using http::DirectoryBody;
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
using option::make_optional;
using std::move;
using std::pair;

auto base_config_from_json(json::Json const& config) -> BaseConfig
{
    if (!config.is_object()) {
        throw std::runtime_error("config is not an object");
    }

    optional<string> config_root;
    optional<vector<string>> config_index_pages;
    optional<map<uint16_t, string>> config_error_pages;
    optional<bool> config_use_cgi;
    optional<vector<HttpMethod>> config_allowed_methods;
    optional<bool> config_autoindex;

    // set root field
    if (config["root"].is_string()) {
        auto root = config["root"].string_value();
        if (!root.ends_with('/'))
            root += '/';
        config_root = std::move(root);
    } else if (!config["root"].is_null()) {
        throw std::runtime_error("http.servers[?].root is not a string");
    }
    TRACEPRINT("root: " << config_root.value_or("(null)"));

    // set index_pages
    if (config["index_pages"].is_array()) {
        vector<string> converted_pages;

        for (auto& page : config["index_pages"].array_items()) {
            if (page.is_string()) {
                converted_pages.push_back(page.string_value());
            } else {
                throw std::runtime_error("http.servers[?].index_pages[?] is not a string");
            }
        }
        config_index_pages = converted_pages;
    } else if (config["index_pages"].is_string()) {
        vector<string> converted_pages;

        converted_pages.push_back(config["index_pages"].string_value());
        config_index_pages = converted_pages;
    } else if (!config["index_pages"].is_null()) {
        throw std::runtime_error("http.servers[?].index_pages is not an array or string");
    }

    // set error pages
    if (config["error_pages"].is_object()) {
        map<uint16_t, string> error_pages;

        for (auto& [status_code_str, page] : config["error_pages"].object_items()) {
            if (!page.is_string()) {
                throw std::runtime_error("http.servers[?].error_pages[?] is not a string");
            }
            // FIXME: atoi
            int code = std::atoi(status_code_str.c_str());
            if (code < 100 || code > 999) {
                throw std::runtime_error("http.servers[?].error_pages[KEY] key is not a valid status code");
            }
            error_pages.insert(std::pair<uint16_t, string>(code, page.string_value()));
        }

        config_error_pages = error_pages;
    } else if (!config["error_pages"].is_null()) {
        throw std::runtime_error("http.servers[?].error_pages is not a map");
    }

    // set use cgi
    if (config["use_cgi"].is_bool()) {
        config_use_cgi = config["use_cgi"].bool_value();
    } else if (!config["use_cgi"].is_null()) {
        throw std::runtime_error("http.servers[?].use_cgi is not a boolean");
    }

    // set allowed methods
    if (config["allowed_methods"].is_array()) {
        vector<HttpMethod> methods;

        for (auto& method : config["allowed_methods"].array_items()) {
            if (!method.is_string()) {
                throw std::runtime_error("allowed_methods contains non-string method");
            }

            const auto& m = method.string_value();
            // lol
            if (m == http::method::CONNECT) {
                methods.push_back(http::method::CONNECT);
            } else if (m == http::method::DELETE) {
                methods.push_back(http::method::DELETE);
            } else if (m == http::method::GET) {
                methods.push_back(http::method::GET);
            } else if (m == http::method::OPTIONS) {
                methods.push_back(http::method::OPTIONS);
            } else if (m == http::method::PATCH) {
                methods.push_back(http::method::PATCH);
            } else if (m == http::method::POST) {
                methods.push_back(http::method::POST);
            } else if (m == http::method::PUT) {
                methods.push_back(http::method::PUT);
            } else {
                throw std::runtime_error("allowed_method contains invalid method");
            }
        }

        config_allowed_methods = methods;
    } else if (!config["allowed_methods"].is_null()) {
        throw std::runtime_error("http.servers[?].allowed_methods is not an array");
    }

    // set autoindex
    if (config["autoindex"].is_bool()) {
        config_autoindex = config["autoindex"].bool_value();
    } else if (!config["autoindex"].is_null()) {
        throw std::runtime_error("http.servers[?].autoindex is not a boolean");
    }

    return BaseConfig(
        move(config_root),
        move(config_index_pages),
        move(config_error_pages),
        move(config_use_cgi),
        move(config_allowed_methods),
        move(config_autoindex));
}

auto recursive_locations(json::Json const& cfg) -> optional<table<Regex, LocationConfig>>
{
    table<Regex, LocationConfig> locations;

    for (auto& [name, obj] : cfg["locations"].object_items()) {
        optional<bool> final = option::nullopt;

        if (obj["final"].is_bool()) {
            final = obj["final"].bool_value();
        } else if (!obj["final"].is_null()) {
            throw std::runtime_error("final field is not a boolean");
        }

        locations.emplace_back(
            std::make_pair(
                Regex(name), LocationConfig(recursive_locations(obj), final, base_config_from_json(obj))));
    }

    // fixes c++ bug
    optional<typeof(locations)> a;

    if (!locations.empty()) {
        a = locations;
    }
    return a;
}

auto config_from_json(json::Json const& config) -> RootConfig
{
    if (!config["http"].is_object())
        throw std::runtime_error("http field is null or not an object");

    optional<uint64_t> worker_count = option::nullopt;

    if (config["workers"].is_number()) {
        int64_t workers = static_cast<int64_t>(config["workers"].number_value());

        if (workers < 0) {
            throw std::runtime_error("worker count is negative");
        }
        worker_count = (uint64_t)workers;
    }
    TRACEPRINT("workers: " << worker_count.value_or(0));

    if (config["http"]["servers"].is_array()) {
        vector<ServerConfig> servers;

        for (auto& server : config["http"]["servers"].array_items()) {
            auto base = base_config_from_json(server);

            optional<vector<string>> server_names = option::nullopt;

            if (server["names"].is_array()) {
                vector<string> names;

                for (auto& name : server["names"].array_items()) {
                    if (!name.is_string()) {
                        throw std::runtime_error("names array contains non-string item");
                    }
                    names.push_back(name.string_value());
                }
                server_names = names;
            } else if (!server["names"].is_null()) {
                throw std::runtime_error("names is not an array");
            }

            optional<vector<tuple<IpAddress, uint16_t>>> bind_addresses = option::nullopt;

            if (server["listen"].is_array()) {
                vector<tuple<IpAddress, uint16_t>> binds;

                for (auto& comboj : server["listen"].array_items()) {
                    if (comboj.is_string()) {
                        auto const& combo = comboj.string_value();

                        std::string_view ip;

                        if (combo.starts_with("[")) {
                            if (combo.find("]:") != std::string::npos) {
                                ip = std::string_view(combo.data() + 1, combo.find("]:") - 1);
                            } else {
                                throw std::runtime_error("listen item is invalid format");
                            }
                        } else {
                            if (combo.find(':') != std::string::npos) {
                                ip = std::string_view(combo.data(), combo.find(':'));
                            } else {
                                throw std::runtime_error("listen item is invalid format");
                            }
                        }

                        auto address = IpAddress::from_str(ip);
                        auto port_str = std::string_view(combo.data() + combo.rfind(':') + 1, combo.length() - combo.rfind(':'));

                        // fixme: atoll
                        std::string sv(port_str);
                        ssize_t port = atoll(sv.c_str());

                        if (port <= 0 || port > UINT16_MAX)
                            throw std::runtime_error("listen item port is zero or out of unsigned 16-bit range");

                        TRACEPRINT("parse ip, port: (" << address << ", " << port << ")");
                        binds.emplace_back(address, port);
                    } else {
                        throw std::runtime_error("listen item is not a string");
                    }
                }

                bind_addresses = binds;
            } else if (!server["bind_addresses"].is_null()) {
                throw std::runtime_error("bind_addresses is not an array");
            }

            optional<table<Regex, LocationConfig>> locations;

            if (server["locations"].is_object()) {
                locations = recursive_locations(server);
            } else if (!server["locations"].is_null()) {
                throw std::runtime_error("server.location is not a map");
            }

            servers.emplace_back(move(server_names), move(bind_addresses), move(locations), std::move(base));
        }

        auto base = base_config_from_json(config["http"]);

        return RootConfig(move(worker_count), HttpConfig(move(servers), move(base)));
    } else {
        throw std::runtime_error("http.servers field is null or not an array");
    }
}

auto match_server(string_view host, vector<ServerConfig> const& servers) -> ServerConfig const&
{
    for (auto const& server : servers) {
        auto const& names = server.get_server_names();

        auto fhost = std::string_view(host);

        {
            auto sc = host.find(':');
            if (host.find(':') != std::string::npos) {
                fhost = std::string_view(host.data(), sc);
            }
        }

        if (names.has_value()) {
            for (auto const& name : *names) {
                if (name == fhost) {
                    TRACEPRINT("host matched servername " << name);
                    return server;
                } else {
                    TRACEPRINT("match failure: " << name << " != " << fhost);
                }
            }
        }
    }
    TRACEPRINT("host match defaulted");
    return servers.at(0);
}

// regex requires C-strings, we can't use string_view
void match_location_i(LocationConfig const& cfg, const char*& path, vector<LocationConfig const*>& abcd)
{
    LocationConfig const* lout = nullptr;
    ptrdiff_t len = -1;

    if (cfg.get_locations().has_value()) {
        for (auto const& [pattern, lcfg] : *cfg.get_locations()) {
            const char* end;

            if (pattern.match(path, &end) != nullptr) {
                ptrdiff_t diff = end - path;
                if (diff > len) {
                    lout = &lcfg;
                    len = diff;
                }
            }
        }
    }
    if (lout != nullptr) {
        DBGPRINT("Location matched: [" << std::string_view(path, len) << "] from (" << path << ")");
        if (!lout->is_final())
            path += len;
        DBGPRINT("Path adjusted: (" << path << ")");
        // TODO: this should probably be a tuple<string, LocationConfig> to not match twice
        abcd.push_back(lout);
        match_location_i(*lout, path, abcd);
    }
    // return lout == nullptr ? std::nullopt : std::optional { lout };
}

auto match_location(LocationConfig const& cfg, http::StreamingHttpRequest& req) -> optional<tuple<vector<LocationConfig const*>, string>>
{
    vector<LocationConfig const*> location_list;
    auto path = req.get_uri().get_pqf()->get_path_escaped().value();
    auto* path_cstr = path.data(); // path happens to point to string

    match_location_i(cfg, path_cstr, location_list);

    // workaround for C++ bug
    optional<tuple<vector<LocationConfig const*>, string>> x;
    if (!location_list.empty()) {
        x = tuple { location_list, string(path_cstr) };
    }
    return x;
}

auto match_base_config(RootConfig const& rcfg, http::StreamingHttpRequest& req) -> tuple<BaseConfig, string>
{
    // FIXME: this could be a lot cleaner
    auto host = req.get_header(http::header::HOST).value_or("");

    auto const& hcfg = rcfg.get_http_config();
    auto const& scfg = match_server(host, hcfg.get_servers());
    auto const& lcfg_o = match_location(scfg, req);
    auto matched_path = req.get_uri().get_pqf()->get_path().value();

    DBGPRINT("mpath " << matched_path );

    optional<string> root;
    optional<vector<string>> index_pages;
    optional<map<uint16_t, string>> error_pages;
    optional<bool> use_cgi;
    optional<vector<HttpMethod>> allowed_methods;
    optional<bool> autoindex;

    if (lcfg_o) {
        auto& [lcfg_v, lcfg_p] = *lcfg_o;

        matched_path = lcfg_p;
        for (auto it = lcfg_v.rbegin(); it != lcfg_v.rend(); ++it) {
            auto& lcfg = *it;

            if (!root && lcfg->get_root()) {
                root = lcfg->get_root();
            }
            if (!index_pages && lcfg->get_index_pages()) {
                index_pages = lcfg->get_index_pages();
            }
            if (!error_pages && lcfg->get_error_pages()) {
                error_pages = lcfg->get_error_pages();
            }
            if (!use_cgi && lcfg->get_use_cgi()) {
                use_cgi = lcfg->get_use_cgi();
            }
            if (!allowed_methods && lcfg->get_allowed_methods()) {
                allowed_methods = lcfg->get_allowed_methods();
            }
            if (!autoindex && lcfg->get_autoindex()) {
                autoindex = lcfg->get_autoindex();
            }
        }
    }

    if (!root && scfg.get_root()) {
        root = scfg.get_root();
    }
    if (!index_pages && scfg.get_index_pages()) {
        index_pages = scfg.get_index_pages();
    }
    if (!error_pages && scfg.get_error_pages()) {
        error_pages = scfg.get_error_pages();
    }
    if (!use_cgi && scfg.get_use_cgi()) {
        use_cgi = scfg.get_use_cgi();
    }
    if (!allowed_methods && scfg.get_allowed_methods()) {
        allowed_methods = scfg.get_allowed_methods();
    }
    if (!autoindex && scfg.get_autoindex()) {
        autoindex = scfg.get_autoindex();
    }
    if (!root && hcfg.get_root()) {
        root = hcfg.get_root();
    }
    if (!index_pages && hcfg.get_index_pages()) {
        index_pages = hcfg.get_index_pages();
    }
    if (!error_pages && hcfg.get_error_pages()) {
        error_pages = hcfg.get_error_pages();
    }
    if (!use_cgi && hcfg.get_use_cgi()) {
        use_cgi = hcfg.get_use_cgi();
    }
    if (!allowed_methods && hcfg.get_allowed_methods()) {
        allowed_methods = hcfg.get_allowed_methods();
    }
    if (!autoindex && hcfg.get_autoindex()) {
        autoindex = hcfg.get_autoindex();
    }
    return tuple {
        BaseConfig(
            move(root),
            move(index_pages),
            move(error_pages),
            move(use_cgi),
            move(allowed_methods),
            move(autoindex)),
        string(matched_path)
    };
}

auto main(int argc, const char** argv) -> int
{
    // ignore SIGPIPE,
    // a dos attack can cause so many SIGPIPEs (to the point of 30k queued),
    // that all threads are preoccupied with running the signal handler
    signal(SIGPIPE, SIG_IGN);
    try {

        std::string config_file_path = "./config.json";

        {
            args::Args arguments(argc, argv);

            // let's just hardcode it instead of using getopt
            auto it = arguments.begin();
            it += 1;
            while (it != arguments.end()) {
                if (*it == "--help" || *it == "-h") {
                    std::cout
                        << "usage: webserv [--config config_file_path]"
                    << std::endl;
                    return 0;
                } else if (*it == "--config" || *it == "-c") {
                    ++it;
                    if (it == arguments.end()) {
                        throw std::runtime_error("expected config file after parameter");
                    } else {
                        // TODO: change working directory to config_file_path's parent directory
                        config_file_path = *it;
                    }
                } else {
                    ERRORPRINT("unknown argument: '" << *it << "'");
                    std::cout
                        << "usage: webserv [-c config_file_path]"
                        << std::endl;
                    return 1;
                }
                ++it;
            }
        }

        json::Json config;
        std::string error;

        {
            auto rt = RuntimeBuilder()
                          .without_workers()
                          .build();

            rt.globalize();

            std::string config_str;
            GlobalRuntime::spawn(ioruntime::FdStringReadFuture(fs::File::open(config_file_path), config_str));
            rt.naive_run();

            config = json::Json::parse(config_str, error);
        }

        if (!error.empty()) {
            ERRORPRINT("bad config: " << error);
            throw std::runtime_error("Config error");
        }

        RootConfigSingleton::set(config_from_json(config));
        auto const& cfg = RootConfigSingleton::get();

        auto builder = RuntimeBuilder();

        {
            auto wc = cfg.get_worker_count().value_or(0);

            if (wc == 0) {
                TRACEPRINT("Building runtime without workers");
                builder = builder.without_workers();
            } else {
                TRACEPRINT("Building runtime with workers (" << wc << ")");
                builder = builder.with_workers(wc);
            }
        }

        auto runtime = builder.build();
        runtime.globalize();

        for (auto& server : cfg.get_http_config().get_servers()) {
            auto bind_addresses = server.get_bind_addresses();
            if (bind_addresses.has_value()) {
                for (auto& [address, port] : *bind_addresses) {
                    GlobalRuntime::spawn(HttpServer(address, port, [](http::StreamingHttpRequest& req) {
                        auto const& cfg = RootConfigSingleton::get();
                        auto host = req.get_header(http::header::HOST);
                        auto method = req.get_method();
                        auto& uri = req.get_uri();

                        auto path = uri.get_pqf()->get_path_escaped().value();

                        auto builder = HttpResponseBuilder();
                        builder.status(http::HTTP_STATUS_NOT_FOUND);

                        const auto [bcfg, matched_path] = match_base_config(cfg, req);
                        {
                            bool method_allowed = false;
                            auto allowed_methods = bcfg.get_allowed_methods();

                            if (allowed_methods.has_value()) {
                                for (auto& allowed_method : *allowed_methods) {
                                    if (allowed_method == method) {
                                        method_allowed = true;
                                        break;
                                    }
                                }
                            } else if (method == http::method::GET || method == http::method::HEAD) {
                                method_allowed = true;
                            }

                            if (!method_allowed) {
                                return std::move(builder).build();
                            }
                        }

                        // find files
                        {
                            auto auto_index = bcfg.get_autoindex().value_or(false);
                            auto const& root = bcfg.get_root();

                            if (root) {
                                auto search_path = string(*root).append(matched_path);
                                try {
                                    auto file = BoxPtr(fs::File::open_no_traversal(search_path));
                                    auto file_size = file->size();
                                    builder.status(http::HTTP_STATUS_OK)
                                        .body(std::move(file), file_size);
                                    return builder.build();
                                } catch (std::invalid_argument& e) {
                                    if (auto_index) {
                                        try {
                                            auto dir = BoxPtr<DirectoryBody>::make(search_path, path);
                                            return HttpResponseBuilder()
                                                .status(http::HTTP_STATUS_OK)
                                                .body(std::move(dir))
                                                .build();
                                        } catch (std::invalid_argument& e) {
                                            // we catch "no such file",
                                            // but we allow other throws
                                            // this way the server will throw 500 on an actual error
                                        }
                                    }
                                    return HttpResponseBuilder()
                                        .status(http::HTTP_STATUS_NOT_FOUND)
                                        .build();
                                }
                            }
                        }
                        return HttpResponseBuilder()
                            .status(http::HTTP_STATUS_NOT_FOUND)
                            .build();
                    }));
                }
            }
        }

        runtime.naive_run();
    } catch (std::exception& e) {
        ERRORPRINT("Failed to load runtime: " << e.what());
    }
    return 0;
}