//
// Created by boat on 08-10-20.
//

#ifndef WEBSERV_CONSTANTS_CONSTANTS_HPP
#define WEBSERV_CONSTANTS_CONSTANTS_HPP

#include <string>

namespace {
using std::string_view;
}


// Software info
constexpr string_view SERVER_SOFTWARE_NAME = "BroServ/latest";
constexpr string_view SERVER_SOFTWARE_METAVAR = "SERVER_NAME=BroServ/latest";

// Config key names
constexpr string_view CONFIG_KEY_ROOT = "root";
constexpr string_view CONFIG_KEY_INDEX = "index";
constexpr string_view CONFIG_KEY_ERROR_PAGES = "error-pages";
constexpr string_view CONFIG_KEY_CGI_ENABLED = "cgi";
constexpr string_view CONFIG_KEY_ALLOWED_METHODS = "allowed-methods";
constexpr string_view CONFIG_KEY_HTTP = "http";
constexpr string_view CONFIG_KEY_WORKERS = "workers";
constexpr string_view CONFIG_KEY_SERVERS = "servers";
constexpr string_view CONFIG_KEY_NAMES = "names";
constexpr string_view CONFIG_KEY_LISTEN = "listen";
constexpr string_view CONFIG_KEY_FINAL = "final";
constexpr string_view CONFIG_KEY_AUTOINDEX = "autoindex";
constexpr string_view CONFIG_KEY_LOCATIONS = "locations";
constexpr string_view CONFIG_KEY_BUFFER_LIMIT = "buffer-limit";
constexpr string_view CONFIG_KEY_BODY_LIMIT = "body-limit";
constexpr string_view CONFIG_KEY_INACTIVITY_TIMEOUT = "timeout";
[[maybe_unused]] constexpr string_view CONFIG_KEY_AUTHENTICATE = "auth";
[[maybe_unused]] constexpr string_view CONFIG_KEY_USERNAME = "username";
[[maybe_unused]] constexpr string_view CONFIG_KEY_PASSWORD = "password";
[[maybe_unused]] constexpr string_view CONFIG_KEY_REALM = "realm";

#endif //WEBSERV_CONSTANTS_CONSTANTS_HPP
