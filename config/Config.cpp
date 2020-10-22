//
// Created by boat on 9/6/20.
//

#include "Config.hpp"
#include <utility>

using option::make_optional;

auto BaseConfig::get_root() const -> optional<string> const&
{
    return root;
}

auto BaseConfig::get_index_pages() const -> optional<vector<string>> const&
{
    return index_pages;
}

auto BaseConfig::get_error_pages() const -> optional<map<uint16_t, string>> const&
{
    return error_pages;
}

auto BaseConfig::get_use_cgi() const -> optional<bool> const&
{
    return use_cgi;
}

auto BaseConfig::get_allowed_methods() const -> optional<vector<Method>> const&
{
    return allowed_methods;
}

auto BaseConfig::get_autoindex() const -> optional<bool> const&
{
    return autoindex;
}

BaseConfig::BaseConfig(optional<string> root, optional<vector<string>> index_pages, optional<map<uint16_t, string>> error_pages, optional<bool> use_cgi, optional<vector<Method>> allowed_methods, optional<bool> autoindex,
    optional<AuthConfig>&& authcfg, optional<UploadConfig>&& upcfg, optional<size_t> body_limit, optional<vector<string>>&& modules)
    : root(std::move(root))
    , index_pages(std::move(index_pages))
    , error_pages(std::move(error_pages))
    , use_cgi(std::move(use_cgi))
    , allowed_methods(std::move(allowed_methods))
    , autoindex(std::move(autoindex))
    , auth_config(std::move(authcfg))
    , upload_config(std::move(upcfg))
    , body_limit(std::move(body_limit))
    , modules(std::move(modules))
{
}

auto BaseConfig::get_auth_config() const -> optional<AuthConfig> const&
{
    return auth_config;
}

auto BaseConfig::get_upload_config() const -> optional<UploadConfig> const&
{
    return upload_config;
}

auto ServerConfig::get_server_names() const -> optional<vector<string>> const&
{
    return server_names;
}

auto ServerConfig::get_bind_addresses() const -> vector<tuple<IpAddress, uint16_t>> const&
{
    return bind_addresses;
}

ServerConfig::ServerConfig(
    optional<vector<string>>&& server_names,
    vector<tuple<IpAddress, uint16_t>>&& bind_addresses,
    optional<table<Regex, LocationConfig>>&& locations,
    optional<size_t> buffer_limit,
    optional<size_t> inactivity_timeout,
    BaseConfig&& bc)
    : LocationConfig(std::move(locations), option::nullopt, std::move(bc))
    , server_names(std::move(server_names))
    , bind_addresses(std::move(bind_addresses))
    , buffer_limit(std::move(buffer_limit))
    , inactivity_timeout(std::move(inactivity_timeout))
{
}

auto BaseConfig::get_body_limit() const -> optional<size_t>
{
    return body_limit;
}

auto BaseConfig::get_modules() const -> optional<vector<string>> const&
{
    return modules;
}

auto ServerConfig::get_buffer_limit() const -> optional<size_t>
{
    return buffer_limit;
}

auto ServerConfig::get_inactivity_timeout() const -> optional<size_t>
{
    return inactivity_timeout;
}

auto LocationConfig::get_locations() const -> optional<table<Regex, LocationConfig>> const&
{
    return locations;
}

auto HttpConfig::get_servers() const -> vector<ServerConfig> const&
{
    return servers;
}
HttpConfig::HttpConfig(vector<ServerConfig>&& servers, BaseConfig&& base)
    : BaseConfig(std::move(base))
    , servers(std::move(servers))
{
}

auto RootConfig::get_worker_count() const -> optional<uint64_t> const&
{
    return worker_count;
}

auto RootConfig::get_http_config() const -> HttpConfig const&
{
    return http_config;
}

RootConfig::RootConfig(optional<uint64_t> worker_count, HttpConfig&& config)
    : worker_count(std::move(worker_count))
    , http_config(std::move(config))
{
}
LocationConfig::LocationConfig(optional<table<Regex, LocationConfig>>&& locations, optional<bool> final, BaseConfig&& base)
    : BaseConfig(std::move(base))
    , locations(std::move(locations))
    , final(final.value_or(false))
{
}
auto LocationConfig::is_final() const -> bool
{
    return final.has_value() && *final;
}

optional<RootConfig> RootConfigSingleton::rc = option::nullopt;

auto RootConfigSingleton::get() -> RootConfig const&
{
    return rc.value();
}

void RootConfigSingleton::set(RootConfig&& rcv)
{
    rc = std::move(rcv);
}

AuthConfig::AuthConfig(string&& username, string&& password, optional<string>&& realm)
    : _username(move(username))
    , _password(move(password))
    , _realm(move(realm))
{
}

auto AuthConfig::get_username() const -> string const&
{
    return _username;
}

auto AuthConfig::get_password() const -> string const&
{
    return _password;
}

auto AuthConfig::get_realm() const -> optional<string> const&
{
    return _realm;
}

UploadConfig::UploadConfig(string&& directory, optional<size_t>&& max_size)
    : _directory(move(directory))
    , _max_size(std::move(max_size))
{
}

auto UploadConfig::get_directory() const -> string const&
{
    return _directory;
}

auto UploadConfig::get_max_size() const -> optional<size_t> const&
{
    return _max_size;
}
