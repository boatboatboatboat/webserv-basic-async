//
// Created by boat on 8/17/20.
//

#ifndef WEBSERV_UTIL_HPP
#define WEBSERV_UTIL_HPP
#include "../http/RfcConstants.hpp"
#include "../option/optional.hpp"
#include "StringStream.hpp"
#include "cstr.hpp"
#include "mem_copy.hpp"
#include "mem_zero.hpp"
#include "span.hpp"
#include <string>

void dbg_puts(std::string const& printme);

#ifdef SAFEPRINT_LINE_INFO
#ifdef __linux__

#include <unistd.h>

#define LINE_INFO __func__ << " " << __FILE__ << ":" << __LINE__ \
                           << " " << std::to_string(gettid())    \
                           << " "
#elif __APPLE__

#define LINE_INFO __func__ << " " << __FILE__ << ":" << __LINE__ \
                           << " "

#endif

#else

#define LINE_INFO ""

#endif

#define SAFEPRINT(x)                           \
    do {                                       \
        try {                                  \
            utils::StringStream __out__;       \
            __out__ << LINE_INFO << x << "\n"; \
            dbg_puts(__out__.str());           \
        } catch (...) {                        \
        }                                      \
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

    size_t old_location = 0;
    while (old_location != std::string::npos) {
        auto new_location = source.find(delimiter);
        strings.push_back(source.substr(old_location, new_location));
        old_location = new_location;
    }
    return strings;
}

inline auto bswap32(uint32_t x) -> uint32_t
{
    return ((((x)&0xff000000u) >> 24u) | (((x)&0x00ff0000u) >> 8u) | (((x)&0x0000ff00u) << 8u) | (((x)&0x000000ffu) << 24u));
}

inline auto bswap32(int32_t x) -> int32_t
{
    return ((((x)&0xff000000) >> 24) | (((x)&0x00ff0000) >> 8) | (((x)&0x0000ff00) << 8) | (((x)&0x000000ff) << 24));
}

inline auto str_eq_case_insensitive(std::string_view a, std::string_view b) -> bool
{
    if (a.length() != b.length())
        return false;
    while (!a.empty() && tolower(a.front()) == tolower(b.front())) {
        a.remove_prefix(1);
        b.remove_prefix(1);
    }
    return a.empty();
}

inline auto string_to_uint64(std::string_view n) -> option::optional<uint64_t>
{
    using namespace http::parser_utils;
    uint64_t num = 0;

    while (!n.empty()) {
        if (!is_digit(n.front())) {
            return option::nullopt;
        }
        uint64_t ofc = num;
        num *= 10;
        if (num < ofc) {
            return option::nullopt;
        }
        num += n.front() - '0';
        n.remove_prefix(1);
    }
    return num;
}

inline auto hexstring_to_uint64(std::string_view n) -> option::optional<uint64_t>
{
    if (n.empty())
        return option::nullopt;

    using namespace http::parser_utils;
    uint64_t num = 0;

    while (!n.empty()) {
        if (!is_hex_digit(n.front())) {
            return option::nullopt;
        }
        uint64_t ofc = num;
        num *= 16;
        if (num < ofc) {
            return option::nullopt;
        }
        num += hex_decode(n.front());
        n.remove_prefix(1);
    }
    return num;
}

inline auto uint64_to_hexstring(uint64_t n) -> std::string
{
    std::string_view charset = "0123456789ABCDEF";
    // do we really care
    std::string s;
    std::string r;

    if (n == 0) {
        s.push_back(charset[n]);
    }
    while (n != 0) {
        s.push_back(charset[n % 16]);
        n /= 16;
    }
    // LOL
    for (auto it = s.rbegin(); it != s.rend(); it++) {
        r.push_back(*it);
    }
    return r;
}

inline auto to_uppercase(char c) -> char
{
    if (c >= 'a' && c <= 'z')
        c ^= 32;
    return c;
}

inline auto to_lowercase(char c) -> char
{
    if (c >= 'A' && c <= 'Z')
        c ^= 32;
    return c;
}

}

#endif //WEBSERV_UTIL_HPP
