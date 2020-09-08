//
// Created by boat on 9/7/20.
//

#ifndef WEBSERV_HTTPMETHOD_HPP
#define WEBSERV_HTTPMETHOD_HPP

#include <string>

namespace http {

using HttpMethod = std::string_view;

namespace method {
    inline constexpr HttpMethod CONNECT = "CONNECT";
    inline constexpr HttpMethod DELETE = "DELETE";
    inline constexpr HttpMethod GET = "GET";
    inline constexpr HttpMethod HEAD = "HEAD";
    inline constexpr HttpMethod OPTIONS = "OPTIONS";
    inline constexpr HttpMethod PATCH = "PATCH";
    inline constexpr HttpMethod POST = "POST";
    inline constexpr HttpMethod PUT = "PUT";
}

}

#endif //WEBSERV_HTTPMETHOD_HPP
