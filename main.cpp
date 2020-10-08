#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "args/Args.hpp"
#include "cgi/Cgi.hpp"
#include "config/Config.hpp"
#include "constants/constants.hpp"
#include "fs/File.hpp"
#include "futures/ForEachFuture.hpp"
#include "futures/SelectFuture.hpp"
#include "http/DirectoryReader.hpp"
#include "http/InfiniteReader.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/Server.hpp"
#include "http/StringReader.hpp"
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
using http::DirectoryReader;
using http::HandlerStatusError;
using http::ResponseBuilder;
using http::Server;
using http::StringReader;
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
    optional<vector<Method>> config_allowed_methods;
    optional<bool> config_autoindex;

    // set root field
    {
        auto key_root = config[CONFIG_KEY_ROOT.data()];
        if (key_root.is_string()) {
            auto root = key_root.string_value();
            if (!root.ends_with('/'))
                root += '/';
            config_root = std::move(root);
        } else if (!key_root.is_null()) {
            throw std::runtime_error("http.servers[?].root is not a string");
        }
        TRACEPRINT("root: " << config_root.value_or("(null)"));
    }
    // set index_pages
    {
        auto index_key = config[CONFIG_KEY_INDEX.data()];
        if (index_key.is_array()) {
            vector<string> converted_pages;

            for (auto& page : index_key.array_items()) {
                if (page.is_string()) {
                    converted_pages.push_back(page.string_value());
                } else {
                    throw std::runtime_error("http.servers[?].index[?] is not a string");
                }
            }
            config_index_pages = converted_pages;
        } else if (index_key.is_string()) {
            vector<string> converted_pages;

            converted_pages.push_back(index_key.string_value());
            config_index_pages = converted_pages;
        } else if (!index_key.is_null()) {
            throw std::runtime_error("http.servers[?].index is not an array or string");
        }
    }
    // set error pages
    {
        auto error_key = config[CONFIG_KEY_ERROR_PAGES.data()];
        if (error_key.is_object()) {
            map<uint16_t, string> error_pages;

            for (auto& [status_code_str, page] : error_key.object_items()) {
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
        } else if (!error_key.is_null()) {
            throw std::runtime_error("http.servers[?].error_pages is not a map");
        }
    }
    // set use cgi
    {
        auto cgi_key = config[CONFIG_KEY_CGI_ENABLED.data()];
        if (cgi_key.is_bool()) {
            config_use_cgi = cgi_key.bool_value();
        } else if (!cgi_key.is_null()) {
            throw std::runtime_error("http.servers[?].use_cgi is not a boolean");
        }
    }
    // set allowed methods
    {
        auto allowed_methods_key = config[CONFIG_KEY_ALLOWED_METHODS.data()];
        if (allowed_methods_key.is_array()) {
            vector<Method> methods;

            for (auto& method : allowed_methods_key.array_items()) {
                if (!method.is_string()) {
                    throw std::runtime_error("allowed-methods contains non-string method");
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
                    throw std::runtime_error("allowed-methods contains invalid method");
                }
            }

            config_allowed_methods = methods;
        } else if (!allowed_methods_key.is_null()) {
            throw std::runtime_error("http.servers[?].allowed_methods is not an array");
        }
    }
    // set autoindex
    {
        auto autoindex_key = config[CONFIG_KEY_AUTOINDEX.data()];
        if (autoindex_key.is_bool()) {
            config_autoindex = autoindex_key.bool_value();
        } else if (!autoindex_key.is_null()) {
            throw std::runtime_error("http.servers[?].autoindex is not a boolean");
        }
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
    auto locations_key = cfg[CONFIG_KEY_LOCATIONS.data()];

    for (auto& [name, obj] : locations_key.object_items()) {
        optional<bool> final = option::nullopt;
        auto final_key = obj[CONFIG_KEY_FINAL.data()];

        if (final_key.is_bool()) {
            final = final_key.bool_value();
        } else if (!final_key.is_null()) {
            throw std::runtime_error("final field is not a boolean");
        }

        locations.emplace_back(
            std::make_pair(
                Regex(name), LocationConfig(recursive_locations(obj), std::move(final), base_config_from_json(obj))));
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
    auto http_key = config[CONFIG_KEY_HTTP.data()];
    if (!http_key.is_object())
        throw std::runtime_error("http field is null or not an object");

    optional<uint64_t> worker_count = option::nullopt;
    {
        auto worker_key = config[CONFIG_KEY_WORKERS.data()];
        if (worker_key.is_number()) {
            int64_t workers = static_cast<int64_t>(worker_key.number_value());

            if (workers < 0) {
                throw std::runtime_error("worker count is negative");
            }
            worker_count = (uint64_t)workers;
        }
        TRACEPRINT("workers: " << worker_count.value_or(0));
    }

    auto server_key = http_key[CONFIG_KEY_SERVERS.data()];
    if (server_key.is_array()) {
        vector<ServerConfig> servers;

        for (auto& server : server_key.array_items()) {
            auto base = base_config_from_json(server);

            optional<vector<string>> server_names = option::nullopt;

            {
                auto name_key = server[CONFIG_KEY_NAMES.data()];
                if (name_key.is_array()) {
                    vector<string> names;

                    for (auto& name : name_key.array_items()) {
                        if (!name.is_string()) {
                            throw std::runtime_error("names array contains non-string item");
                        }
                        names.push_back(name.string_value());
                    }
                    server_names = names;
                } else if (!name_key.is_null()) {
                    throw std::runtime_error("names is not an array");
                }
            }
            optional<vector<tuple<IpAddress, uint16_t>>> bind_addresses = option::nullopt;
            {
                auto listen_key = server[CONFIG_KEY_LISTEN.data()];
                if (listen_key.is_array()) {
                    vector<tuple<IpAddress, uint16_t>> binds;

                    for (auto& comboj : listen_key.array_items()) {
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
                } else if (!listen_key.is_null()) {
                    throw std::runtime_error("listen is not an array");
                }
            }
            optional<table<Regex, LocationConfig>> locations;

            {
                auto locations_key = server[CONFIG_KEY_LOCATIONS.data()];
                if (locations_key.is_object()) {
                    locations = recursive_locations(server);
                } else if (!locations_key.is_null()) {
                    throw std::runtime_error("server.location is not a map");
                }
            }

            optional<size_t> body_limit;
            {
                auto body_limit_key = server[CONFIG_KEY_BODY_LIMIT.data()];
                if (body_limit_key.is_number()) {
                    auto lim = body_limit_key.number_value();
                    if ((lim - 1.0) > (double)SIZE_MAX) {
                        throw std::runtime_error("body limit is greater than SIZE_MAx");
                    } else if (lim < 1) {
                        throw std::runtime_error("body limit is less than one");
                    }
                    body_limit = lim;
                } else if (!body_limit_key.is_null()) {
                    throw std::runtime_error("body limit is not a number");
                }
            }

            optional<size_t> buffer_limit;
            {
                auto buffer_limit_key = server[CONFIG_KEY_BUFFER_LIMIT.data()];
                if (buffer_limit_key.is_number()) {
                    auto lim = buffer_limit_key.number_value();
                    if ((lim - 1.0) > (double)SIZE_MAX) {
                        throw std::runtime_error("buffer limit is greater than SIZE_MAx");
                    } else if (lim < 1) {
                        throw std::runtime_error("buffer limit is less than one");
                    }
                    buffer_limit = (size_t)lim;
                } else if (!buffer_limit_key.is_null()) {
                    throw std::runtime_error("buffer limit is not a number");
                }
            }

            optional<size_t> inactivity_timeout;
            {
                auto timeout_key = server[CONFIG_KEY_INACTIVITY_TIMEOUT.data()];
                if (timeout_key.is_number()) {
                    auto lim = timeout_key.number_value();
                    if ((lim - 1.0) > (double)SIZE_MAX) {
                        throw std::runtime_error("timeout is greater than SIZE_MAx");
                    } else if (lim < 1) {
                        throw std::runtime_error("timeout is less than one");
                    }
                    inactivity_timeout = (size_t)lim;
                } else if (!timeout_key.is_null()) {
                    throw std::runtime_error("timeout is not a number");
                }
            }

            servers.emplace_back(move(server_names), move(bind_addresses),
                move(locations),
                move(body_limit), move(buffer_limit), move(inactivity_timeout),
                move(base));
        }

        auto base = base_config_from_json(http_key);

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
        TRACEPRINT("Location matched: [" << std::string_view(path, len) << "] from (" << path << ")");
        if (!lout->is_final())
            path += len;
        TRACEPRINT("Path adjusted: (" << path << ")");
        // TODO: this should probably be a tuple<string, LocationConfig> to not match twice
        abcd.push_back(lout);
        match_location_i(*lout, path, abcd);
    }
    // return lout == nullptr ? std::nullopt : std::optional { lout };
}

auto match_location(LocationConfig const& cfg, http::Request& req) -> optional<tuple<vector<LocationConfig const*>, string>>
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

auto match_base_config(RootConfig const& rcfg, http::Request& req) -> tuple<BaseConfig, string>
{
    // FIXME: this could be a lot cleaner
    auto host = req.get_header(http::header::HOST).value_or("");

    auto const& hcfg = rcfg.get_http_config();
    auto const& scfg = match_server(host, hcfg.get_servers());
    auto const& lcfg_o = match_location(scfg, req);

    auto matched_path = std::get<1>(lcfg_o.value());

    TRACEPRINT("matched path " << matched_path);

    optional<string> root;
    optional<vector<string>> index_pages;
    optional<map<uint16_t, string>> error_pages;
    optional<bool> use_cgi;
    optional<vector<Method>> allowed_methods;
    optional<bool> autoindex;

    if (lcfg_o.has_value()) {
        auto& [lcfg_v, lcfg_p] = *lcfg_o;

        matched_path = lcfg_p;
        for (auto it = lcfg_v.rbegin(); it != lcfg_v.rend(); ++it) {
            auto& lcfg = *it;

            if (!root.has_value() && lcfg->get_root().has_value()) {
                root = lcfg->get_root();
            }
            if (!index_pages.has_value() && lcfg->get_index_pages().has_value()) {
                index_pages = lcfg->get_index_pages();
            }
            if (!error_pages.has_value() && lcfg->get_error_pages().has_value()) {
                error_pages = lcfg->get_error_pages();
            }
            if (!use_cgi.has_value() && lcfg->get_use_cgi().has_value()) {
                use_cgi = lcfg->get_use_cgi();
            }
            if (!allowed_methods.has_value() && lcfg->get_allowed_methods().has_value()) {
                allowed_methods = lcfg->get_allowed_methods();
            }
            if (!autoindex.has_value() && lcfg->get_autoindex().has_value()) {
                autoindex = lcfg->get_autoindex();
            }
        }
    }

    if (!root.has_value() && scfg.get_root().has_value()) {
        root = scfg.get_root();
    }
    if (!index_pages.has_value() && scfg.get_index_pages().has_value()) {
        index_pages = scfg.get_index_pages();
    }
    if (!error_pages.has_value() && scfg.get_error_pages().has_value()) {
        error_pages = scfg.get_error_pages();
    }
    if (!use_cgi.has_value() && scfg.get_use_cgi().has_value()) {
        use_cgi = scfg.get_use_cgi();
    }
    if (!allowed_methods.has_value() && scfg.get_allowed_methods().has_value()) {
        allowed_methods = scfg.get_allowed_methods();
    }
    if (!autoindex.has_value() && scfg.get_autoindex().has_value()) {
        autoindex = scfg.get_autoindex();
    }
    if (!root.has_value() && hcfg.get_root().has_value()) {
        root = hcfg.get_root();
    }
    if (!index_pages.has_value() && hcfg.get_index_pages().has_value()) {
        index_pages = hcfg.get_index_pages();
    }
    if (!error_pages.has_value() && hcfg.get_error_pages().has_value()) {
        error_pages = hcfg.get_error_pages();
    }
    if (!use_cgi.has_value() && hcfg.get_use_cgi().has_value()) {
        use_cgi = hcfg.get_use_cgi();
    }
    if (!allowed_methods.has_value() && hcfg.get_allowed_methods().has_value()) {
        allowed_methods = hcfg.get_allowed_methods();
    }
    if (!autoindex.has_value() && hcfg.get_autoindex().has_value()) {
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
                        throw std::runtime_error("expected config file after parameter '-c'");
                    } else {
                        // TODO: change working directory to config_file_path's parent directory
                        auto parent_directory_end = config_file_path.rfind('/');
                        if (parent_directory_end != string::npos) {
                            // there's a /, so chdir
                            auto parent_directory = config_file_path.substr(parent_directory_end + 1);

                            if (chdir(parent_directory.c_str()) < 0) {
                                ERRORPRINT("bad directory for '-c'");
                                std::cout
                                    << "usage: webserv [-c config_file_path]"
                                    << std::endl;
                                return 1;
                            };
                        }
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
                    auto service =
                        [&cfg = std::as_const(cfg), server_address = address, server_port = port](
                            http::Request& req, net::SocketAddr const& socket_addr) {
                            (void)server_address;
                            (void)server_port;
                            auto host = req.get_header(http::header::HOST);
                            auto method = req.get_method();
                            auto& uri = req.get_uri();

                            auto path = uri.get_pqf()->get_path_escaped().value();

                            auto builder = ResponseBuilder();
                            builder.status(http::status::OK);

                            const auto [bcfg, matched_path] = match_base_config(cfg, req);
                            {
                                // set Allow header, check allowed methods
                                bool method_allowed = false;
                                auto allowed_methods = bcfg.get_allowed_methods();

                                std::stringstream allow_field_value;
                                http::Headers allow_header;

                                if (allowed_methods.has_value()) {
                                    bool first = false;
                                    for (auto& allowed_method : *allowed_methods) {
                                        if (first) {
                                            allow_field_value << ", ";
                                        } else {
                                            first = true;
                                        }
                                        allow_field_value << allowed_method;
                                        if (allowed_method == method) {
                                            method_allowed = true;
                                        }
                                    }
                                } else if (method == http::method::GET || method == http::method::HEAD) {
                                    // default to allowed methods GET and HEAD
                                    method_allowed = true;
                                    allow_field_value << "GET, HEAD";
                                }

                                allow_header.push_back(http::Header { http::header::ALLOW, allow_field_value.str() });
                                if (!method_allowed) {
                                    throw HandlerStatusError(http::status::METHOD_NOT_ALLOWED, move(allow_header));
                                }

                                builder.headers(move(allow_header));
                            }
                            // set software name
                            builder.header(http::header::SERVER, string(SERVER_SOFTWARE_NAME));

                            // find files
                            {
                                auto auto_index = bcfg.get_autoindex().value_or(false);
                                auto const& root = bcfg.get_root();

                                if (root.has_value()) {
                                    auto search_path = string(*root).append(matched_path);
                                    auto use_cgi = bcfg.get_use_cgi();

                                    try {
                                        if (use_cgi.has_value() && *use_cgi) {
                                            // This is (for now) ran for its error side effects
                                            // TODO: write a better file-exists check
                                            (void)fs::File::open_no_traversal(search_path);
                                            // it already executes here.
                                            auto cgi_body = cgi::Cgi(search_path, move(req), socket_addr);
                                            builder.cgi(move(cgi_body));
                                        } else {
                                            auto file = BoxPtr(fs::File::open_no_traversal(search_path));
                                            auto file_size = file->size();
                                            builder.body(move(file), file_size);
                                        }
                                        return move(builder).build();
                                    } catch (fs::FileIsDirectory& e) {
                                        if (auto_index) {
                                            auto dir = BoxPtr<DirectoryReader>::make(search_path, path);
                                            builder.body(move(dir));
                                            return move(builder).build();
                                        } else if (bcfg.get_index_pages().has_value()) {
                                            auto& pages = *bcfg.get_index_pages();
                                            for (auto& page_name : pages) {
                                                try {
                                                    auto file = BoxPtr(fs::File::open_no_traversal(search_path + page_name));
                                                    auto file_size = file->size();
                                                    builder.body(std::move(file), file_size);
                                                    return move(builder).build();
                                                } catch (fs::FileError& e) {
                                                    /* don't do anything if the file isn't found/is directory */
                                                }
                                            }
                                            throw fs::FileNotFound();
                                        } else {
                                            throw fs::FileNotFound();
                                        }
                                    }
                                }
                            }
                            throw fs::FileNotFound();
                        };
                    GlobalRuntime::spawn(
                        Server::bind(address, port)
                            .body_limit(server.get_body_limit().value_or(size_t(http::DEFAULT_BODY_LIMIT)))
                            .buffer_limit(server.get_buffer_limit().value_or(size_t(http::DEFAULT_BUFFER_LIMIT)))
                            .inactivity_timeout(server.get_inactivity_timeout().value_or(size_t(5000)))
                            .serve(service));
                }
            }
        }

        runtime.naive_run();
    } catch (std::exception& e) {
        ERRORPRINT("Failed to load runtime: " << e.what());
    }
    return 0;
}