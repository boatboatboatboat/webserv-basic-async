//
// Created by boat on 17-10-20.
//

#include "localtime.hpp"
#include <string>

namespace utils {

auto get_http_date(time_t curtime) -> std::string {
    char date_str[128];
    struct tm _tm {
    };
    std::string out;

    int written = strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S ", not_localtime_r(&curtime, &_tm));
    if (written < 0) {
        throw std::runtime_error(strerror(errno));
    }
    out = std::string(date_str, written);
    // nullptr timezone
    out += "UTC";
    return out;
}

auto get_http_date_now() -> std::string
{
    struct timeval tv {
    };
    gettimeofday(&tv, nullptr);
    return get_http_date(tv.tv_sec);
}

}