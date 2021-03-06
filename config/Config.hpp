//
// Created by boat on 9/6/20.
//

#ifndef WEBSERV_CONFIG_HPP
#define WEBSERV_CONFIG_HPP

#include "../http/Method.hpp"
#include "../net/IpAddress.hpp"
#include "../option/optional.hpp"
#include "../regex/Regex.hpp"
#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

using option::optional;
using std::map;
using std::pair;
using std::string;
using std::tuple;
using std::vector;

template <typename K, typename V>
using table = vector<pair<K, V>>;

using http::Method;
using net::IpAddress;

class AuthConfig {
public:
    AuthConfig(string&& username, string&& password, optional<string>&& realm);
    [[nodiscard]] auto get_username() const -> string const&;
    [[nodiscard]] auto get_password() const -> string const&;
    [[nodiscard]] auto get_realm() const -> optional<string> const&;

private:
    string _username;
    string _password;
    optional<string> _realm;
};

class UploadConfig {
public:
    UploadConfig(string&& directory, optional<size_t>&& max_size);
    [[nodiscard]] auto get_directory() const -> string const&;
    [[nodiscard]] auto get_max_size() const -> optional<size_t> const&;

private:
    string _directory;
    optional<size_t> _max_size;
};

class BaseConfig {
public:
    BaseConfig(
        optional<string> root,
        optional<vector<string>> index_pages,
        optional<map<uint16_t, string>> error_pages,
        optional<bool> use_cgi,
        optional<vector<Method>> allowed_methods,
        optional<bool> autoindex,
        optional<AuthConfig>&& authcfg,
        optional<UploadConfig>&& uploadcfg,
        optional<size_t> body_limit,
        optional<vector<string>>&& modules,
        optional<tuple<IpAddress, uint16_t>>&& proxy);
    [[nodiscard]] auto get_root() const -> optional<string> const&;
    [[nodiscard]] auto get_index_pages() const -> optional<vector<string>> const&;
    [[nodiscard]] auto get_error_pages() const -> optional<map<uint16_t, string>> const&;
    [[nodiscard]] auto get_use_cgi() const -> optional<bool> const&;
    [[nodiscard]] auto get_allowed_methods() const -> optional<vector<Method>> const&;
    [[nodiscard]] auto get_autoindex() const -> optional<bool> const&;
    [[nodiscard]] auto get_auth_config() const -> optional<AuthConfig> const&;
    [[nodiscard]] auto get_upload_config() const -> optional<UploadConfig> const&;
    [[nodiscard]] auto get_body_limit() const -> optional<size_t>;
    [[nodiscard]] auto get_modules() const -> optional<vector<string>> const&;
    [[nodiscard]] auto get_proxy() const -> optional<tuple<IpAddress, uint16_t>> const&;

private:
    optional<string> root;
    optional<vector<string>> index_pages;
    optional<map<uint16_t, string>> error_pages;
    optional<bool> use_cgi;
    optional<vector<Method>> allowed_methods;
    optional<bool> autoindex;
    optional<AuthConfig> auth_config;
    optional<UploadConfig> upload_config;
    optional<size_t> body_limit;
    optional<vector<string>> modules;
    optional<tuple<IpAddress, uint16_t>> proxy;
};

class LocationConfig : public BaseConfig {
public:
    explicit LocationConfig(optional<table<Regex, LocationConfig>>&&, optional<bool> final, BaseConfig&& base);
    [[nodiscard]] auto get_locations() const -> optional<table<Regex, LocationConfig>> const&;
    [[nodiscard]] auto is_final() const -> bool;

private:
    optional<table<Regex, LocationConfig>> locations;
    optional<bool> final;
};

class ServerConfig : public LocationConfig {
public:
    ServerConfig(
        optional<vector<string>>&& server_names,
        vector<tuple<IpAddress, uint16_t>>&& bind_addresses,
        optional<table<Regex, LocationConfig>>&& locations,
        optional<size_t> buffer_limit,
        optional<size_t> inactivity_timeout,
        BaseConfig&& bc);
    [[nodiscard]] auto get_server_names() const -> optional<vector<string>> const&;
    [[nodiscard]] auto get_bind_addresses() const -> vector<tuple<IpAddress, uint16_t>> const&;
    [[nodiscard]] auto get_buffer_limit() const -> optional<size_t>;
    [[nodiscard]] auto get_inactivity_timeout() const -> optional<size_t>;

private:
    optional<vector<string>> server_names;
    vector<tuple<IpAddress, uint16_t>> bind_addresses;
    optional<size_t> buffer_limit;
    optional<size_t> inactivity_timeout;
};

class HttpConfig : public BaseConfig {
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
