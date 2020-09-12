//
// Created by boat on 9/6/20.
//

#ifndef WEBSERV_CONFIG_HPP
#define WEBSERV_CONFIG_HPP


#include <cstdint>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include "../net/IpAddress.hpp"
#include "../http/HttpMethod.hpp"
#include "../regex/Regex.hpp"

using std::optional;
using std::vector;
using std::string;
using std::tuple;
using std::map;
using std::pair;

template<typename K, typename V>
using table = vector<pair<K, V>>;

using net::IpAddress;
using http::HttpMethod;

class BaseConfig {
public:
    BaseConfig(optional<string> root, optional<vector<string>> index_pages, optional<map<uint16_t, string>> error_pages, optional<bool> use_cgi, optional<vector<HttpMethod>> allowed_methods, optional<bool> autoindex);
    [[nodiscard]] auto get_root() const -> optional<string> const&;
    [[nodiscard]] auto get_index_pages() const -> optional<vector<string>> const&;
    [[nodiscard]] auto get_error_pages() const -> optional<map<uint16_t, string>> const&;
    [[nodiscard]] auto get_use_cgi() const -> optional<bool> const&;
    [[nodiscard]] auto get_allowed_methods() const -> optional<vector<HttpMethod>> const&;
    [[nodiscard]] auto get_autoindex() const -> optional<bool> const&;
private:
    optional<string> root;
    optional<vector<string>> index_pages;
    optional<map<uint16_t, string>> error_pages;
    optional<bool> use_cgi;
    optional<vector<HttpMethod>> allowed_methods;
    optional<bool> autoindex;
};

class LocationConfig: public BaseConfig {
public:
    explicit LocationConfig(optional<table<Regex, LocationConfig>>&&, BaseConfig&& base);
    [[nodiscard]] auto get_locations() const -> optional<table<Regex, LocationConfig>> const&;
private:
    optional<table<Regex, LocationConfig>> locations;
};

class ServerConfig: public LocationConfig {
public:
    ServerConfig(optional<vector<string>>&& server_names, optional<vector<tuple<IpAddress, uint16_t>>>&& bind_addresses, optional<table<Regex, LocationConfig>>&& locations, BaseConfig&& bc);
    [[nodiscard]] auto get_server_names() const -> optional<vector<string>> const&;
    [[nodiscard]] auto get_bind_addresses() const -> optional<vector<tuple<IpAddress, uint16_t>>> const&;
private:
    optional<vector<string>> server_names;
    optional<vector<tuple<IpAddress, uint16_t>>> bind_addresses;
};

class HttpConfig: public BaseConfig {
public:
    HttpConfig(vector<ServerConfig>&& servers, BaseConfig&& base);
    [[nodiscard]] auto get_servers() const -> vector<ServerConfig> const&;
private:
    vector<ServerConfig> servers;
};

class RootConfig {
public:
    RootConfig(optional<uint64_t> worker_count, HttpConfig&& config);
    [[nodiscard]] auto get_worker_count() const -> optional<uint64_t> const&;
    [[nodiscard]] auto get_http_config() const -> HttpConfig const&;
private:
    optional<uint64_t> worker_count;
    HttpConfig http_config;
};

class RootConfigSingleton {
public:
    static auto get() -> RootConfig const&;
    static void set(RootConfig&& rcv);
private:
    static optional<RootConfig> rc;
};

#endif //WEBSERV_CONFIG_HPP
