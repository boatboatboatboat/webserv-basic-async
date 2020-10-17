//
// Created by boat on 8/27/20.
//

#ifndef WEBSERV_HTTPRFCCONSTANTS_HPP
#define WEBSERV_HTTPRFCCONSTANTS_HPP

#include <algorithm>
#include <string>

namespace http {

inline std::string_view SP = " ";
inline std::string_view CLRF = "\r\n";

namespace parser_utils {
    using std::string_view;
    inline auto is_alpha_lower(char c) -> bool
    {
        return c >= 'a' && c <= 'z';
    }

    inline auto is_control(char c) -> bool
    {
        return (c >= 0 && c <= 31) || c == 127;
    }

    inline auto is_lws(char c) -> bool
    {
        return c == ' ' || c == '\t';
    }

    inline auto is_text(char c) -> bool {
        return !is_control(c) || is_lws(c);
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

    inline auto is_alphanumeric(char c) -> bool
    {
        return is_alpha(c) || is_digit(c);
    }

    inline auto is_unreserved(char c) -> bool
    {
        return is_alphanumeric(c) || c == '-' || c == '.' || c == '_' || c == '~';
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

    inline auto is_tchar(char c)
    {
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

    inline auto is_vchar(char c)
    {
        return c >= 0x21 && c <= 0x7e;
    }

    inline void parse_expect(string_view& sv, string_view c)
    {
        if (sv.empty()) {
            throw std::runtime_error("parser: unexpected eof");
        }
        auto cur = sv.front();
        if (c.find(cur) == string_view::npos) {
            throw std::runtime_error("parser: unexpected char");
        }
        sv.remove_prefix(1);
    }

    inline void parse_until(string_view& sv, string_view c, size_t minimum = 0, size_t maximum = -1)
    {
        size_t total = 0;

        for (;;) {
            if (sv.empty()) {
                throw std::runtime_error("parser: expected char");
            }
            auto cur = sv.front();
            if (c.find(cur) == string_view::npos) {
                total += 1;
            } else {
                break;
            }
        }
        if (total > maximum) {
            throw std::runtime_error("parser: above maximum");
        }
        if (total < minimum) {
            throw std::runtime_error("parser: below minimum");
        }
    }

    inline void parse_while(string_view& sv, string_view c, size_t minimum = 0, size_t maximum = -1)
    {
        size_t total = 0;

        for (;;) {
            if (sv.empty()) {
                break;
            }
            auto cur = sv.front();
            if (c.find(cur) == string_view::npos) {
                break;
            } else {
                total += 1;
            }
        }
        if (total > maximum) {
            throw std::runtime_error("parser: above maximum");
        }
        if (total < minimum) {
            throw std::runtime_error("parser: below minimum");
        }
    }

    inline void parse_while(string_view& sv, bool (*c)(char), size_t minimum = 0, size_t maximum = -1)
    {
        size_t total = 0;

        for (;;) {
            if (sv.empty()) {
                break;
            }
            auto cur = sv.front();
            if (c(cur)) {
                total += 1;
            } else {
                break;
            }
            sv.remove_prefix(1);
        }
        if (total > maximum) {
            throw std::runtime_error("parser: above maximum");
        }
        if (total < minimum) {
            throw std::runtime_error("parser: below minimum");
        }
    }

    inline auto parse_optional(string_view& sv, string_view c) -> bool
    {
        if (sv.empty()) {
            return false;
        }
        auto cur = sv.front();
        if (c.find(cur) == string_view::npos) {
            return false;
        }
        sv.remove_prefix(1);
        return true;
    }

    inline auto parse_optional(string_view& sv, bool (*c)(char)) -> bool
    {
        if (sv.empty()) {
            return false;
        }
        auto cur = sv.front();
        if (c(cur)) {
            return false;
        }
        sv.remove_prefix(1);
        return true;
    }

    inline void parse_look_expect(string_view& sv, string_view c)
    {
        if (sv.empty()) {
            throw std::runtime_error("parser: unexpected eof");
        }
        auto cur = sv.front();
        if (c.find(cur) == string_view::npos) {
            throw std::runtime_error("parser: unexpected char");
        }
    }

    inline auto parse_look_optional(string_view& sv, string_view c) -> bool
    {
        if (sv.empty()) {
            return false;
        }
        auto cur = sv.front();
        if (c.find(cur) == string_view::npos) {
            return false;
        }
        return true;
    }

}

}

#endif //WEBSERV_HTTPRFCCONSTANTS_HPP
