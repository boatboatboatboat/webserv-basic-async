//
// Created by boat on 8/26/20.
//

#ifndef WEBSERV_HTTPVERSION_HPP
#define WEBSERV_HTTPVERSION_HPP

#include <iostream>

namespace http {

struct HttpVersion {
    std::string_view version_string;
};

namespace version {
    inline constexpr HttpVersion v1_0 { "HTTP/1.0" };
    inline constexpr HttpVersion v1_1 { "HTTP/1.1" };
}

}

std::ostream& operator<<(std::ostream& os, http::HttpVersion const& version);

#endif //WEBSERV_HTTPVERSION_HPP
