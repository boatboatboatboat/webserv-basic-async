//
// Created by boat on 9/26/20.
//

#ifndef WEBSERV_HTTP_URI_HPP
#define WEBSERV_HTTP_URI_HPP

#include "../option/optional.hpp"
#include <algorithm>
#include <string>

namespace http {

using option::optional;
using std::string;
using std::string_view;

namespace uri_parser_utils {
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
}

// StandardSchemes are "already known" schemes (http, https)
using StandardScheme = string_view;
// CustomSchemes are "unknown" schemes (ftp, irc, ...)
using CustomScheme = string;

class Scheme {
public:
    class SchemeParseError : public std::runtime_error {
    public:
        SchemeParseError();
    };

    Scheme();
    auto operator=(Scheme&& other) noexcept -> Scheme&;
    Scheme(Scheme&& other) noexcept;
    explicit Scheme(string_view view);
    ~Scheme();

    [[nodiscard]] auto is_standard() const -> bool;
    [[nodiscard]] auto is_none() const -> bool;
    [[nodiscard]] auto is_custom() const -> bool;
    [[nodiscard]] auto get() const -> string_view;
private:
    enum SchemeType {
        None,
        Standard,
        Custom
    } _type
        = None;
    union {
        StandardScheme _standard;
        CustomScheme _custom;
    };
};

class Authority {
public:
    class AuthorityParseError : public std::runtime_error {
    public:
        AuthorityParseError();
    };
    explicit Authority(string_view str);

    [[nodiscard]] auto get_userinfo() const -> optional<string> const&;
    [[nodiscard]] auto get_host() const -> string const&;
    [[nodiscard]] auto get_port() const -> optional<uint16_t>;
private:
    optional<string> _userinfo;
    string _host;
    optional<uint16_t> _port;
};

class PathQueryFragment {
public:
    class PathQueryFragmentError : public std::runtime_error {
    public:
        PathQueryFragmentError();
    };
    explicit PathQueryFragment(string_view str);

    [[nodiscard]] auto get_path() const -> optional<string_view>;
    [[nodiscard]] auto get_path_escaped() const -> optional<string_view>;
    [[nodiscard]] auto get_query() const -> optional<string_view>;
    [[nodiscard]] auto get_fragment() const -> optional<string_view>;
private:
    struct Path {
        string _escaped;
        optional<string> _unescaped;
    };
    optional<Path> _path = option::nullopt;
    optional<string> _query = option::nullopt;
    optional<string> _fragment = option::nullopt;
};

using Query = string;
using Path = string;
using Fragment = string;

class Uri {
public:
    class UriParseError : public std::runtime_error {
    public:
        UriParseError();
    };
    enum RequestTargetForm {
        OriginForm,
        AbsoluteForm,
        AuthorityForm,
        AsteriskForm
    };

    Uri() = delete;
    explicit Uri(string_view str);

    [[nodiscard]] auto get_target_form() const -> RequestTargetForm;
    [[nodiscard]] auto get_scheme() const -> optional<Scheme> const&;
    [[nodiscard]] auto get_authority() const -> optional<Authority> const&;
    [[nodiscard]] auto get_pqf() const -> optional<PathQueryFragment> const&;
private:
    RequestTargetForm _target_form;
    optional<Scheme> _scheme = option::nullopt;
    optional<Authority> _authority = option::nullopt;
    optional<PathQueryFragment> _pqf = option::nullopt;
};

}

#endif //WEBSERV_HTTP_URI_HPP
