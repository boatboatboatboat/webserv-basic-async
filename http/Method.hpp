//
// Created by boat on 9/7/20.
//

#ifndef WEBSERV_HTTPMETHOD_HPP
#define WEBSERV_HTTPMETHOD_HPP

#include <string>

namespace http {

using Method = std::string_view;

namespace method {
    constexpr Method CONNECT = "CONNECT";
    constexpr Method DELETE = "DELETE";
    constexpr Method GET = "GET";
    constexpr Method HEAD = "HEAD";
    constexpr Method OPTIONS = "OPTIONS";
    constexpr Method PATCH = "PATCH";
    constexpr Method POST = "POST";
    constexpr Method PUT = "PUT";
}

}

#endif //WEBSERV_HTTPMETHOD_HPP
