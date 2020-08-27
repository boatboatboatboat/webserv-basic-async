//
// Created by boat on 8/26/20.
//

#ifndef WEBSERV_HTTPVERSION_HPP
#define WEBSERV_HTTPVERSION_HPP

namespace http {

struct HttpVersion {
    const char* version_string;
};

extern const HttpVersion HTTP_VERSION_1_0;
extern const HttpVersion HTTP_VERSION_1_1;

}

#endif //WEBSERV_HTTPVERSION_HPP
