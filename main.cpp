#include "ioruntime/ioruntime.hpp"
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>

#include "args/Args.hpp"
#include "cgi/Cgi.hpp"
#include "config/Config.hpp"
#include "constants/config.hpp"
#include "constants/mimetypes.hpp"
#include "fs/File.hpp"
#include "fs/Path.hpp"
#include "futures/ForEachFuture.hpp"
#include "http/DirectoryReader.hpp"
#include "http/FutureSeReader.hpp"
#include "http/IncomingRequest.hpp"
#include "http/OutgoingResponse.hpp"
#include "http/Server.hpp"
#include "ioruntime/FdStringReadFuture.hpp"
#include "langmod/ESLanguage.hpp"
#include "langmod/LuaLanguage.hpp"
#include "net/SocketAddr.hpp"
#include "regex/Regex.hpp"
#include "utils/base64.hpp"
#include "utils/localtime.hpp"
#include "utils/monostate.hpp"
#include "utils/utils.hpp"
#include "json/Json.hpp"
#include <csignal>

using futures::ForEachFuture;
using futures::IFuture;
using futures::PollResult;
using http::DirectoryReader;
using http::HandlerStatusError;
using http::OutgoingResponseBuilder;
using http::Server;
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
    optional<AuthConfig> config_auth;
    optional<UploadConfig> config_upload;
    optional<vector<string>> config_modules;

    // set root field
    {
        auto key_root = config[CONFIG_KEY_ROOT.data()];
        if (key_root.is_string()) {
            auto root = key_root.string_value();
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
                auto code = utils::string_to_uint64(status_code_str);
                if (!code.has_value() || *code < 100 || *code > 999) {
                    throw std::runtime_error("http.servers[?].error_pages[KEY] key is not a valid status code");
                }
                error_pages.insert(std::pair<uint16_t, string>(*code, page.string_value()));
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
                } else if (m == http::method::HEAD) {
                    methods.push_back(http::method::HEAD);
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

    // set auth
    {
        auto auth_key = config[CONFIG_KEY_AUTHENTICATE.data()];
        if (auth_key.is_object()) {
            auto username_key = auth_key[CONFIG_KEY_USERNAME.data()];
            optional<string> config_username;
            if (username_key.is_string()) {
                config_username = string(username_key.string_value());
            } else {
                throw std::runtime_error("auth.username is not a string");
            }
            auto password_key = auth_key[CONFIG_KEY_PASSWORD.data()];
            optional<string> config_password;
            if (password_key.is_string()) {
                config_password = string(password_key.string_value());
            } else {
                throw std::runtime_error("auth.password is not a string");
            }
            auto realm_key = auth_key[CONFIG_KEY_REALM.data()];
            optional<string> config_realm;
            if (realm_key.is_string()) {
                auto sv = realm_key.string_value();
                std::stringstream realmbuilder;
                for (char c : sv) {
                    if (c == '\\' || c == '"') {
                        realmbuilder << '\\';
                    }
                    realmbuilder << c;
                }

                config_realm = realmbuilder.str();
            } else if (!realm_key.is_null()) {
                throw std::runtime_error("auth.realm is not a string");
            }
            config_auth = AuthConfig(move(*config_username), move(*config_password), move(config_realm));
        } else if (!auth_key.is_null()) {
            throw std::runtime_error("base.auth is not an object");
        }
    }

    // set upload
    {
        auto upload_key = config[CONFIG_KEY_UPLOAD.data()];
        if (upload_key.is_object()) {
            auto directory_key = upload_key[CONFIG_KEY_UPLOAD_DIRECTORY.data()];
            optional<string> config_directory;
            if (directory_key.is_string()) {
                config_directory = string(directory_key.string_value());
            } else {
                throw std::runtime_error("base.upload.directory is not an object");
            }
            auto max_size_key = upload_key[CONFIG_KEY_UPLOAD_MAX_SIZE.data()];
            optional<size_t> config_max_size;
            if (max_size_key.is_number()) {
                auto as_double = max_size_key.number_value();
                auto as_size_t = (size_t)as_double;
                if (as_double < 0) {
                    throw std::runtime_error("base.upload.max_size is negative");
                }
                if (as_double >= (double)UINT64_MAX) {
                    throw std::runtime_error("base.upload.max_size is too big");
                }
                if (as_size_t != 0) {
                    config_max_size = as_size_t;
                }
            } else if (max_size_key.is_null()) {
                config_max_size = size_t(CONFIG_DEFAULT_UPLOAD_MAX_SIZE);
            } else {
                throw std::runtime_error("base.upload.max_size is not a number");
            }
            config_upload = UploadConfig(move(*config_directory), move(config_max_size));
        } else if (!upload_key.is_null()) {
            throw std::runtime_error("base.upload is not an object");
        }
    }

    optional<size_t> body_limit;
    {
        auto body_limit_key = config[CONFIG_KEY_BODY_LIMIT.data()];
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

    {
        auto modules_key = config[CONFIG_KEY_MODULES.data()];
        if (modules_key.is_array()) {
            vector<string> items;

            for (auto const& item : modules_key.array_items()) {
                if (!item.is_string()) {
                    throw std::runtime_error("modules item is not a string");
                }
                items.push_back(item.string_value());
            }
            config_modules = move(items);
        } else if (!modules_key.is_null()) {
            throw std::runtime_error("modules is not an array");
        }
    }

    return BaseConfig(
        move(config_root),
        move(config_index_pages),
        move(config_error_pages),
        move(config_use_cgi),
        move(config_allowed_methods),
        move(config_autoindex),
        move(config_auth),
        move(config_upload),
        move(body_limit),
        move(config_modules));
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

                            std::string sv(port_str);
                            auto port = utils::string_to_uint64(sv);

                            if (!port.has_value() || *port > UINT16_MAX)
                                throw std::runtime_error("listen item port is zero or out of unsigned 16-bit range");

                            TRACEPRINT("parse ip, port: (" << address << ", " << *port << ")");
                            binds.emplace_back(address, *port);
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

            servers.emplace_back(move(server_names), move(*bind_addresses),
                move(locations),
                move(buffer_limit), move(inactivity_timeout),
                move(base));
        }

        auto base = base_config_from_json(http_key);

        return RootConfig(move(worker_count), HttpConfig(move(servers), move(base)));
    } else {
        throw std::runtime_error("http.servers field is null or not an array");
    }
}

auto match_server(IpAddress const& me, unsigned short port, string_view host, vector<ServerConfig> const& servers) -> ServerConfig const&
{
    ServerConfig const* default_server = nullptr;
    for (auto const& server : servers) {
        bool listener_matched = false;

        for (auto const& listener : server.get_bind_addresses()) {
            auto const& [ip, sport] = listener;

            if (me == ip && port == sport) {
                listener_matched = true;
                if (default_server == nullptr) {
                    default_server = &server;
                }
                break;
            }
        }

        if (!listener_matched) {
            continue;
        }

        auto fhost = std::string_view(host);
        {
            auto sc = host.find(':');
            if (host.find(':') != std::string::npos) {
                fhost = std::string_view(host.data(), sc);
            }
        }

        auto const& names = server.get_server_names();
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
    return *default_server;
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
        abcd.push_back(lout);
        match_location_i(*lout, path, abcd);
    }
}

auto match_location(LocationConfig const& cfg, http::IncomingRequest& req) -> optional<tuple<vector<LocationConfig const*>, string>>
{
    vector<LocationConfig const*> location_list;
    const auto& pqf = req.get_uri().get_pqf();
    string_view path = "/";
    if (pqf.has_value()) {
        path = pqf->get_path_escaped().value_or("/");
    }
    auto* path_cstr = path.data(); // path happens to point to string

    match_location_i(cfg, path_cstr, location_list);

    // workaround for C++ issue
    optional<tuple<vector<LocationConfig const*>, string>> x;
    if (!location_list.empty()) {
        x = tuple { location_list, string(path_cstr) };
    }
    return x;
}

auto match_base_config(IpAddress const& addr, unsigned short port, RootConfig const& rcfg, http::IncomingRequest& req) -> tuple<BaseConfig, string>
{
    // FIXME: this could be a lot cleaner
    auto host = req.get_message().get_header(http::header::HOST).value_or("");

    auto const& hcfg = rcfg.get_http_config();
    auto const& scfg = match_server(addr, port, host, hcfg.get_servers());
    auto const& lcfg_o = match_location(scfg, req);

    if (!lcfg_o.has_value()) {
        // eh
        throw fs::FileNotFound();
    }
    auto matched_path = std::get<1>(lcfg_o.value());

    TRACEPRINT("matched path " << matched_path);

    optional<string> root;
    optional<vector<string>> index_pages;
    optional<map<uint16_t, string>> error_pages;
    optional<bool> use_cgi;
    optional<vector<Method>> allowed_methods;
    optional<bool> autoindex;
    optional<AuthConfig> authcfg;
    optional<UploadConfig> upcfg;
    optional<size_t> body_limit;
    optional<vector<string>> modules;

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
            if (!authcfg.has_value() && lcfg->get_auth_config().has_value()) {
                authcfg = lcfg->get_auth_config();
            }
            if (!upcfg.has_value() && lcfg->get_upload_config().has_value()) {
                upcfg = lcfg->get_upload_config();
            }
            if (!body_limit.has_value() && lcfg->get_body_limit().has_value()) {
                body_limit = lcfg->get_body_limit();
            }
            if (!modules.has_value() && lcfg->get_modules().has_value()) {
                modules = lcfg->get_modules();
            }
        }
    }

    // SCFG
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
    if (!authcfg.has_value() && scfg.get_auth_config().has_value()) {
        authcfg = scfg.get_auth_config();
    }
    if (!upcfg.has_value() && scfg.get_upload_config().has_value()) {
        upcfg = scfg.get_upload_config();
    }
    if (!body_limit.has_value() && scfg.get_body_limit().has_value()) {
        body_limit = scfg.get_body_limit();
    }
    if (!modules.has_value() && scfg.get_modules().has_value()) {
        modules = scfg.get_modules();
    }
    // HCFG
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
    if (!authcfg.has_value() && hcfg.get_auth_config().has_value()) {
        authcfg = hcfg.get_auth_config();
    }
    if (!upcfg.has_value() && hcfg.get_upload_config().has_value()) {
        upcfg = hcfg.get_upload_config();
    }
    if (!body_limit.has_value() && hcfg.get_body_limit().has_value()) {
        body_limit = hcfg.get_body_limit();
    }
    if (!modules.has_value() && hcfg.get_modules().has_value()) {
        modules = hcfg.get_modules();
    }
    return tuple {
        BaseConfig(
            move(root),
            move(index_pages),
            move(error_pages),
            move(use_cgi),
            move(allowed_methods),
            move(autoindex),
            move(authcfg),
            move(upcfg),
            move(body_limit),
            move(modules)),
        string(matched_path)
    };
}

inline static void process_cl_args(args::Args arguments, string& config_file_path)
{
    // let's just hardcode it instead of using getopt
    auto it = arguments.begin();
    it += 1;
    while (it != arguments.end()) {
        if (*it == "--help" || *it == "-h") {
            std::cout
                << "usage: webserv [--config config_file_path]"
                << std::endl;
            exit(0);
        } else if (*it == "--config" || *it == "-c") {
            ++it;
            if (it == arguments.end()) {
                throw std::runtime_error("expected config file after parameter '-c'");
            } else {
                const auto& argument = *it;
                const auto parent_directory_end = argument.rfind('/');
                if (parent_directory_end != string::npos) {
                    // there's a /, so chdir
                    const auto parent_directory = string(argument.substr(0, parent_directory_end));

                    if (chdir(parent_directory.c_str()) < 0) {
                        ERRORPRINT("bad directory for '-c', (" << parent_directory << ")");
                        std::cout
                            << "usage: webserv [-c config_file_path]"
                            << std::endl;
                        exit(1);
                    };
                }
                config_file_path = argument.substr(parent_directory_end + 1);
            }
        } else {
            ERRORPRINT("unknown argument: '" << *it << "'");
            std::cout
                << "usage: webserv [-c config_file_path]"
                << std::endl;
            exit(1);
        }
        ++it;
    }
}

inline static void check_method_and_set_allow(BaseConfig const& bcfg, Method method, OutgoingResponseBuilder& builder)
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

inline static auto load_config_from_path(std::string const& path) -> json::Json
{
    auto rt = RuntimeBuilder()
                  .without_workers()
                  .build();
    rt.globalize();

    std::string config_str;
    GlobalRuntime::spawn(ioruntime::FdStringReadFuture(fs::File::open(path), config_str));
    rt.naive_run();

    std::string error;

    auto cfg = json::Json::parse(config_str, error);

    if (!error.empty()) {
        ERRORPRINT("failed to load config: " << path);
        throw std::runtime_error("bad config");
    }

    return cfg;
}

inline static void check_inner_body_limit(optional<http::IncomingBody> const& body, BaseConfig const& bcfg)
{
    if (body.has_value() && bcfg.get_body_limit().has_value()) {
        if (body->size() > *bcfg.get_body_limit()) {
            throw HandlerStatusError(http::status::PAYLOAD_TOO_LARGE);
        }
    }
}

inline static auto is_request_upload(http::Method method, BaseConfig const& bcfg) -> bool
{
    return method == http::method::PUT && bcfg.get_upload_config().has_value();
}

inline static auto try_download(
    http::OutgoingResponseBuilder& builder,
    std::string_view base_path,
    std::string const& matched_path,
    optional<http::IncomingBody>& body,
    BaseConfig const& bcfg) -> bool
{
    // method is PUT and upload_config is set
    const auto& upload_config = *bcfg.get_upload_config();
    auto upload_directory_info = fs::Path::path(upload_config.get_directory());
    auto concat_info = fs::Path::path(upload_config.get_directory() + "/" + matched_path);

    if (concat_info.exists() && concat_info.is_directory()) {
        return false;
    }
    if (upload_directory_info.exists() && upload_directory_info.is_directory()) {
        // this is vulnerable to some toctou bs
        // but do we really care? blame the sysadmin
        string concatenated = upload_config.get_directory();
        concatenated += "/";
        concatenated += matched_path;
        auto file = fs::File::create_no_traversal(concatenated);
        if (body.has_value()) {
            auto& span_reader = *body;
            auto copy_future = ioruntime::IoCopyFuture<typeof(span_reader), typeof(file)>(move(span_reader), move(file));
            builder
                .status(http::status::CREATED)
                .header(http::header::CONTENT_LOCATION, string(base_path)) // RFC 7231-3.1.4.2.
                .header(http::header::LOCATION, string(base_path))
                .body(BoxPtr<http::FutureSeReader<typeof(copy_future)>>::make(move(copy_future)));
        }
        return true;
    }
    return false;
}

inline void check_authorization(http::IncomingRequest const& req, BaseConfig const& cfg)
{
    auto auth_enabled = cfg.get_auth_config();
    if (!auth_enabled.has_value()) {
        return;
    }

    auto exit_reason = http::status::UNAUTHORIZED;
    auto auth_header = req.get_message().get_header(http::header::AUTHORIZATION);

    if (auth_header.has_value()) {
        auto const& auth_value = *auth_header;
        // credentials field can't be empty because of the : requirement
        if (auth_value.starts_with("Basic ")) {
            auto encoded = auth_value.substr(6);
            string decoded;
            try {
                decoded = utils::base64::decode_string(encoded);
            } catch (utils::base64::Base64DecodeError const&) {
                exit_reason = http::status::BAD_REQUEST;
            }
            auto user_end = decoded.find(':');
            if (user_end != std::string::npos) {
                auto user = decoded.substr(0, user_end);
                auto password = decoded.substr(user_end + 1);
                if (std::all_of(user.begin(), user.end(), http::parser_utils::is_text)) {
                    if (std::all_of(password.begin(), password.end(), http::parser_utils::is_text)) {
                        volatile auto x = utils::safe_streq(user, auth_enabled->get_username());
                        x = x & utils::safe_streq(password, auth_enabled->get_password());
                        if (x) {
                            return;
                        }
                    } else {
                        exit_reason = http::status::BAD_REQUEST;
                    }
                } else {
                    exit_reason = http::status::BAD_REQUEST;
                }
            } else {
                exit_reason = http::status::BAD_REQUEST;
            }
        }
    }
    http::Headers www_auth;
    www_auth.push_back(http::Header { http::header::WWW_AUTHENTICATE, "Basic realm=\"" + auth_enabled->get_realm().value_or("none") + "\"" });
    throw HandlerStatusError(exit_reason, move(www_auth));
}

inline auto is_proxy_request(BaseConfig const& bcfg) -> bool {
    (void)bcfg;
    return false;
}

auto main(int argc, const char** argv) -> int
{
    // ignore SIGPIPE,
    // a dos attack can cause so many SIGPIPEs (to the point of 30k queued),
    // that all threads are preoccupied with running the signal handler
    signal(SIGPIPE, SIG_IGN);
    try {
        std::string config_file_path = "./config.json";
        process_cl_args(args::Args(argc, argv), config_file_path);

        auto config = load_config_from_path(config_file_path);
        auto const& cfg = config_from_json(config);

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
            std::map<std::tuple<net::IpAddress, unsigned short>, utils::monostate> used_addresses;
            for (auto const& bind_address : bind_addresses) {
                if (used_addresses.find(bind_address) != used_addresses.end()) {
                    // we already started this server
                    continue;
                }
                used_addresses.emplace(bind_address, utils::monostate());
                auto const& [address, port] = bind_address;
                auto service =
                    [&cfg = std::as_const(cfg), server_address = address, server_port = port](
                        http::IncomingRequest& req, net::SocketAddr const& socket_addr) {
                        if (req.get_version().version_string != http::version::v1_1.version_string) {
                            // we're http/1.1, we don't care about the other ones
                            throw HandlerStatusError(http::status::VERSION_NOT_SUPPORTED);
                        }
                        auto const& message = req.get_message();
                        auto const& host = *message.get_header(http::header::HOST);
                        auto const& method = req.get_method();
                        auto const& uri = req.get_uri();
                        auto const& pqf = uri.get_pqf();
                        auto& body = req.get_message().get_body();

                        string_view path = "/";
                        if (pqf.has_value()) {
                            path = pqf->get_path_escaped().value_or("/");
                        }
                        string hv_location(path);
                        string hv_content_location(path);

                        auto builder = OutgoingResponseBuilder();
                        builder
                            .status(http::status::OK)
                            .header(http::header::CONNECTION, "close");
                        const auto [bcfg, matched_path] = match_base_config(server_address, server_port, cfg, req);
                        // check if authorized
                        check_authorization(req, bcfg);
                        // set allow header
                        check_method_and_set_allow(bcfg, method, builder);
                        // set software name header
                        builder
                            .header(http::header::SERVER, string(SERVER_SOFTWARE_NAME))
                            .header(http::header::DATE, utils::get_http_date_now());
                        // check "inner" body limit
                        check_inner_body_limit(body, bcfg);
                        if (is_proxy_request(bcfg)) {

                        } else
                        if (is_request_upload(method, bcfg)) {
                            if (try_download(builder, path, matched_path, body, bcfg)) {
                                return move(builder).build();
                            } else {
                                throw fs::FileNotFound();
                            }
                        } else {
                            // Find and serve files
                            auto auto_index = bcfg.get_autoindex().value_or(false);
                            auto const& root = bcfg.get_root();

                            if (!root.has_value()) {
                                throw fs::FileNotFound();
                            }

                            auto search_path = string(*root);
                            auto root_path_info = fs::Path::no_traversal(*root);

                            if (!root_path_info.exists()) {
                                throw fs::FileNotFound();
                            } else if (root_path_info.is_directory()) {
                                if (!search_path.ends_with('/')) {
                                    search_path += '/';
                                }
                            }
                            search_path.append(matched_path);

                            auto search_path_info = fs::Path::no_traversal(search_path);
                            if (!search_path_info.exists()) {
                                throw fs::FileNotFound();
                            } else if (search_path_info.is_directory()) {
                                if (auto_index) {
                                    auto directory = BoxPtr<DirectoryReader>::make(search_path, path);
                                    return move(builder
                                                    .header(http::header::CONTENT_ENCODING, "text/html")
                                                    .header(http::header::LOCATION, string(path))
                                                    .header(http::header::LAST_MODIFIED, utils::get_http_date(*search_path_info.get_last_modification_time()))
                                                    .body(move(directory)))
                                        .build();
                                } else {
                                    if (!search_path.ends_with('/')) {
                                        search_path += '/';
                                    }
                                    if (!hv_content_location.ends_with('/')) {
                                        hv_content_location += '/';
                                    }
                                    // find first matching index
                                    auto index_pages = bcfg.get_index_pages();
                                    bool matched = false;
                                    if (index_pages.has_value()) {
                                        for (auto const& filename : *index_pages) {
                                            auto file_path = search_path + filename;
                                            auto index_path_info = fs::Path::path(file_path);
                                            if (index_path_info.exists() && index_path_info.is_file()) {
                                                matched = true;
                                                search_path = move(file_path);
                                                hv_content_location += filename;
                                                break;
                                            }
                                        }
                                    }
                                    if (!matched) {
                                        throw fs::FileNotFound();
                                    }
                                }
                            }

                            auto fhost = std::string_view(host);
                            {
                                auto sc = host.find(':');
                                if (host.find(':') != std::string::npos) {
                                    fhost = std::string_view(host.data(), sc);
                                }
                            }
                            auto is_cgi_enabled = bcfg.get_use_cgi().value_or(false);
                            auto const& modules = bcfg.get_modules();
                            auto module_response = false;
                            if (modules.has_value() && !modules->empty()) {
                                // completely "dynamic" modules
                                for (auto const& module : *modules) {
                                    if (module_response) {
                                        // do nothing - response has already been set
                                    } else if (module == "lua") {
                                        cgi::CgiServerForwardInfo csfi {
                                            .sockaddr = socket_addr,
                                            .server_name = fhost,
                                            .server_port = server_port,
                                        };
                                        builder.cgi(cgi::Cgi(search_path, move(req), move(csfi), langmod::LuaLanguage()));
                                        module_response = true; // :-)
                                    } else if (module == "es") {
                                        cgi::CgiServerForwardInfo csfi {
                                            .sockaddr = socket_addr,
                                            .server_name = fhost,
                                            .server_port = server_port,
                                        };
                                        builder.cgi(cgi::Cgi(search_path, move(req), move(csfi), langmod::ESLanguage()));
                                        module_response = true; // :-)
                                    }
                                }
                            }
                            if (module_response) {
                                // do nothing - response has already been set
                            } else if (is_cgi_enabled) {
                                cgi::CgiServerForwardInfo csfi {
                                    .sockaddr = socket_addr,
                                    .server_name = fhost,
                                    .server_port = server_port,
                                };
                                builder.cgi(cgi::Cgi(search_path, move(req), move(csfi)));
                            } else {
                                auto file_info = fs::Path::no_traversal(search_path);
                                auto const& file_ext = file_info.get_extension();
                                if (file_ext.has_value()) {
                                    auto mime_type = constants::get_mime_type(*file_ext);
                                    builder.header(http::header::CONTENT_TYPE, string(mime_type))
                                        .header(http::header::CONTENT_LOCATION, hv_content_location)
                                        .header(http::header::LAST_MODIFIED, utils::get_http_date(*file_info.get_last_modification_time()))
                                        .header(http::header::LOCATION, hv_location);
                                }
                                auto file = BoxPtr(fs::File::open_no_traversal(search_path));
                                auto file_size = file->size();
                                builder.body(move(file), file_size);
                            }
                            return move(builder).build();
                        }
                        throw fs::FileNotFound();
                    };
                auto error_page_handler = [&cfg = std::as_const(cfg)](http::Status const& status) {
                    auto const& pages = cfg.get_http_config().get_error_pages();
                    if (pages.has_value()) {
                        try {
                            for (const auto& [code, file] : *pages) {
                                if (status.code == code) {
                                    return BoxPtr<IAsyncRead>(BoxPtr(fs::File::open(file)));
                                }
                            }
                        } catch (...) {
                            // fallthrough
                        }
                    }
                    return BoxPtr<IAsyncRead>(BoxPtr<http::DefaultPageReader>::make(status));
                };
                GlobalRuntime::spawn(
                    Server::bind(address, port)
                        .body_limit(server.get_body_limit().value_or(size_t(http::DEFAULT_BODY_LIMIT)))
                        .buffer_limit(server.get_buffer_limit().value_or(size_t(http::DEFAULT_BUFFER_LIMIT)))
                        .inactivity_timeout(server.get_inactivity_timeout().value_or(size_t(5000)))
                        .error_page_handler(error_page_handler)
                        .serve(service));
            }
        }

        runtime.naive_run();
    } catch (std::exception& e) {
        ERRORPRINT("Failed to load runtime: " << e.what());
    }
    return 0;
}