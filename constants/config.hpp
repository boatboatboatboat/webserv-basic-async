//
// Created by boat on 08-10-20.
//

#ifndef WEBSERV_CONSTANTS_CONFIG_HPP
#define WEBSERV_CONSTANTS_CONFIG_HPP

#include <string>

namespace {
using std::string_view;
}

// Software info
constexpr string_view SERVER_SOFTWARE_NAME = "BroServ/latest";
constexpr string_view SERVER_SOFTWARE_METAVAR = "SERVER_SOFTWARE=BroServ/latest";

// Root config key names
constexpr string_view CONFIG_KEY_HTTP = "http";
constexpr string_view CONFIG_KEY_WORKERS = "workers";

// Http config key names
constexpr string_view CONFIG_KEY_SERVERS = "servers";

// Base config key names
constexpr string_view CONFIG_KEY_ROOT = "root";
constexpr string_view CONFIG_KEY_INDEX = "index";
constexpr string_view CONFIG_KEY_ERROR_PAGES = "error-pages";
constexpr string_view CONFIG_KEY_CGI_ENABLED = "cgi";
constexpr string_view CONFIG_KEY_ALLOWED_METHODS = "allowed-methods";
constexpr string_view CONFIG_KEY_FINAL = "final";
constexpr string_view CONFIG_KEY_AUTOINDEX = "autoindex";
constexpr string_view CONFIG_KEY_LOCATIONS = "locations";

// Server config key names
constexpr string_view CONFIG_KEY_BUFFER_LIMIT = "buffer-limit";
constexpr string_view CONFIG_KEY_BODY_LIMIT = "body-limit";
constexpr string_view CONFIG_KEY_INACTIVITY_TIMEOUT = "timeout";
constexpr string_view CONFIG_KEY_NAMES = "names";
constexpr string_view CONFIG_KEY_LISTEN = "listen";

// Authentication config key names
constexpr string_view CONFIG_KEY_AUTHENTICATE = "auth";
constexpr string_view CONFIG_KEY_USERNAME = "username";
constexpr string_view CONFIG_KEY_PASSWORD = "password";
constexpr string_view CONFIG_KEY_REALM = "realm";

// Upload config key names
constexpr string_view CONFIG_KEY_UPLOAD = "upload";
constexpr string_view CONFIG_KEY_UPLOAD_DIRECTORY = "directory";
constexpr string_view CONFIG_KEY_UPLOAD_MAX_SIZE = "max-size";

// Module key names
constexpr string_view CONFIG_KEY_MODULES = "modules";
constexpr string_view MODULE_LUA = "lua";
constexpr string_view MODULE_ECMASCRIPT = "es";

// Proxy key names
constexpr string_view CONFIG_KEY_PROXY = "proxy";

// Config default values
constexpr size_t CONFIG_DEFAULT_UPLOAD_MAX_SIZE = 100000;

#endif //WEBSERV_CONSTANTS_CONFIG_HPP
