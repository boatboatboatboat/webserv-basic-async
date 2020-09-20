//
// Created by boat on 9/7/20.
//

#ifndef WEBSERV_HTTPMETHOD_HPP
#define WEBSERV_HTTPMETHOD_HPP

#include <string>

namespace http {

using HttpMethod = std::string_view;

namespace method {
    constexpr HttpMethod CONNECT = "CONNECT";
    constexpr HttpMethod DELETE = "DELETE";
    constexpr HttpMethod GET = "GET";
    constexpr HttpMethod HEAD = "HEAD";
    constexpr HttpMethod OPTIONS = "OPTIONS";
    constexpr HttpMethod PATCH = "PATCH";
    constexpr HttpMethod POST = "POST";
    constexpr HttpMethod PUT = "PUT";
}

}

#endif //WEBSERV_HTTPMETHOD_HPP
