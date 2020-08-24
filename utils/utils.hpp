//
// Created by boat on 8/17/20.
//

#ifndef WEBSERV_UTIL_HPP
#define WEBSERV_UTIL_HPP
#include "mem_copy.hpp"
#include <string>

void dbg_puts(std::string const& printme);

#ifdef SAFEPRINT_LINE_INFO
#ifdef __linux__

#define LINE_INFO      __func__ << " " << __FILE__ << ":" << __LINE__ \
                                << " " << std::to_string(gettid())    \
                                << " "
#elif __APPLE__

#define LINE_INFO  __func__ << " " << __FILE__ << ":" << __LINE__ \
                                << " "

#endif

#else

#define LINE_INFO ""

#endif

#define SAFEPRINT(x)                         \
    do {                                     \
        try {                                \
            std::stringstream __out__;       \
            __out__ << LINE_INFO << x << "\n"; \
            dbg_puts(__out__.str());         \
        } catch (...) {                      \
        }                                    \
    } while (0)

#if LOG_DEBUG || LOG_TRACE

#define DBGPRINT(x) SAFEPRINT("\033[32mdebug\033[0m: " << x)

#else

#define DBGPRINT(x)

#endif

#if LOG_TRACE

#define TRACEPRINT(x) SAFEPRINT("\033[35mtrace\033[0m: " << x)

#else

#define TRACEPRINT(x)

#endif

#if LOG_INFO || LOG_DEBUG || LOG_TRACE

#define INFOPRINT(x) SAFEPRINT("\033[36minfo\033[0m: " << x)

#else

#define INFOPRINT(x)

#endif

#if LOG_WARN || LOG_INFO || LOG_DEBUG || LOG_TRACE

#define WARNPRINT(x) SAFEPRINT("\033[33mwarn\033[0m: " << x)

#else

#define WARNPRINT(x)

#endif

#if LOG_ERROR || LOG_WARN || LOG_INFO || LOG_DEBUG || LOG_TRACE

#define ERRORPRINT(x) SAFEPRINT("\033[31merror\033[0m: " << x)

#else

#define ERRORPRINT(x)

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
inline std::string trim(const std::string& str)
{
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
inline std::string toLower(std::string& value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value;
}

/**
 * Split a string on a given delimiter.
 * @param source the string to split.
 * @param delimiter the delimiter to split on.
 * @return the delimited string in a vector.
 */
inline std::vector<std::string> split(const std::string& source, char delimiter)
{
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
