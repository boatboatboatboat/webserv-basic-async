//
// Created by boat on 8/17/20.
//

#ifndef WEBSERV_UTIL_HPP
#define WEBSERV_UTIL_HPP
#include "mem_copy.hpp"
#include <string>

void dbg_puts(std::string const& printme);

#ifdef __linux__

// gettid() only works on linux

#define DBGPRINT(x)                      \
    do {                                 \
        std::string __x(__func__);       \
        __x += (" ");                    \
        __x.append(__FILE__);            \
        __x.append(":");                 \
        __x += std::to_string(__LINE__); \
        __x.append(" ");                 \
        __x += std::to_string(gettid()); \
        __x.append(" ");                 \
        __x.append(x);                   \
        __x.append("\n");                \
        dbg_puts(__x);                   \
    } while (0)

#endif

#ifdef __APPLE__

#define DBGPRINT(x)                      \
    do {                                 \
        std::string __x(__func__);       \
        __x += (" ");                    \
        __x.append(__FILE__);            \
        __x.append(":");                 \
        __x += std::to_string(__LINE__); \
        __x.append(" ");                 \
        __x.append(x);                   \
        __x.append("\n");                \
        dbg_puts(__x);                   \
    } while (0)

#endif

// FIXME: illegal header
#include <sstream>
#include <vector>

namespace utils {
/**
 * Trim a string.
 * @param str the string to trim.
 * @return the trimmed string.
 */
inline std::string trim(const std::string & str) {
    auto first = str.find_first_not_of(' ');

    if (std::string::npos == first)
        return str;

    auto last = str.find_last_not_of(' ');

    return str.substr(first, (last - first + 1));
}

/**
 * Lowercase a string.
 * @param value a reference to the string to lowercase.
 * @return the lowercase string.
 */
inline std::string toLower(std::string & value) {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value;
}

/**
 * Split a string on a given delimiter.
 * @param source the string to split.
 * @param delimiter the delimiter to split on.
 * @return the delimited string in a vector.
 */
inline std::vector<std::string> split(const std::string & source, char delimiter) {
    std::vector<std::string> strings;
    std::istringstream iss(source);
    std::string s;

    while (std::getline(iss, s, delimiter)) {
        strings.push_back(utils::trim(s));
    }

    return strings;
}
}

#endif //WEBSERV_UTIL_HPP
