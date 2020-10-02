//
// Created by boat on 8/27/20.
//

#ifndef WEBSERV_HTTPRFCCONSTANTS_HPP
#define WEBSERV_HTTPRFCCONSTANTS_HPP

#include <string>

namespace http {

inline std::string_view SP = " ";
inline std::string_view CLRF = "\r\n";

namespace parser_utils {
inline auto is_alpha_lower(char c) -> bool
{
    return c >= 'a' && c <= 'z';
}

inline auto is_alpha_upper(char c) -> bool
{
    return c >= 'A' && c <= 'Z';
}

inline auto is_alpha(char c) -> bool
{
    return is_alpha_lower(c) || is_alpha_upper(c);
}

inline auto is_digit(char c) -> bool
{
    return c >= '0' && c <= '9';
}

inline auto is_unreserved(char c) -> bool
{
    return is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

inline auto is_gen_delims(char c) -> bool
{
    switch (c) {
    case ':':
    case '/':
    case '?':
    case '#':
    case '[':
    case ']':
    case '@':
        return true;
    default:
        return false;
    }
}

inline auto is_sub_delims(char c) -> bool
{
    switch (c) {
    case '!':
    case '$':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case ';':
    case '=':
        return true;
    default:
        return false;
    }
}

inline auto is_reserved(char c) -> bool
{
    return is_gen_delims(c) || is_sub_delims(c);
}

inline auto is_hex_digit(char c) -> bool
{
    return is_digit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline auto is_pct_encoded(string_view str) -> bool
{
    if (str.length() != 3) {
        return false;
    }
    return str[0] == '%' && is_hex_digit(str[1]) && is_hex_digit(str[2]);
}

// returns 17 on invalid value
inline auto hex_decode(char c) -> uint8_t
{
    if (is_digit(c)) {
        return c - '0';
    } else if (is_alpha_lower(c)) {
        return c - 'a' + 10;
    } else if (is_alpha_upper(c)) {
        return c - 'A' + 10;
    }
    return 17;
}

// behaviour is undefined if str is not a valid pct-encoded
inline auto pct_decode(string_view str) -> char
{
    return hex_decode(str[1]) * 0x10 + hex_decode(str[2]);
}

inline auto is_tchar(char c) {
    if (is_digit(c) || is_alpha(c))
        return true;
    switch (c) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '_':
    case '`':
    case '|':
    case '~':
        return true;
    default:
        return false;
    }
}

}

}

#endif //WEBSERV_HTTPRFCCONSTANTS_HPP
