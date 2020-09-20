//
// Created by boat on 9/6/20.
//

#include "Config.hpp"

#include <utility>

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

auto BaseConfig::get_allowed_methods() const -> optional<vector<HttpMethod>> const&
{
    return allowed_methods;
}

auto BaseConfig::get_autoindex() const -> optional<bool> const&
{
    return autoindex;
}
BaseConfig::BaseConfig(optional<string> root, optional<vector<string>> index_pages, optional<map<uint16_t, string>> error_pages, optional<bool> use_cgi, optional<vector<HttpMethod>> allowed_methods, optional<bool> autoindex)
    : root(std::move(root))
    , index_pages(std::move(index_pages))
    , error_pages(std::move(error_pages))
    , use_cgi(use_cgi)
    , allowed_methods(std::move(allowed_methods))
    , autoindex(autoindex)
{
}

auto ServerConfig::get_server_names() const -> optional<vector<string>> const&
{
    return server_names;
}

auto ServerConfig::get_bind_addresses() const -> optional<vector<tuple<IpAddress, uint16_t>>> const&
{
    return bind_addresses;
}

ServerConfig::ServerConfig(optional<vector<string>>&& server_names, optional<vector<tuple<IpAddress, uint16_t>>>&& bind_addresses, optional<table<Regex, LocationConfig>>&& locations, BaseConfig&& bc)
    : LocationConfig(std::move(locations), std::nullopt, std::move(bc))
    , server_names(std::move(server_names))
    , bind_addresses(std::move(bind_addresses))
{
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
    : worker_count(worker_count)
    , http_config(std::move(config))
{
}
LocationConfig::LocationConfig(optional<table<Regex, LocationConfig>>&& locations, optional<bool> final, BaseConfig&& base)
    : BaseConfig(std::move(base))
    , locations(std::move(locations))
    , final(final)
{
}
auto LocationConfig::is_final() const -> bool
{
    return final && *final;
}

optional<RootConfig> RootConfigSingleton::rc = std::nullopt;

auto RootConfigSingleton::get() -> RootConfig const&
{
    return rc.value();
}

void RootConfigSingleton::set(RootConfig&& rcv)
{
    rc = std::optional { std::move(rcv) };
}
