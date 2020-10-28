//
// Created by boat on 9/26/20.
//

#ifndef WEBSERV_HTTP_URI_HPP
#define WEBSERV_HTTP_URI_HPP

#include "../option/optional.hpp"
#include "../utils/StringStream.hpp"
#include <algorithm>
#include <string>

namespace http {

using option::optional;
using std::string;
using std::string_view;

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
    Scheme(Scheme&& other) noexcept;
    Scheme(Scheme const& other);
    auto operator=(Scheme&& other) noexcept -> Scheme&;
    auto operator=(Scheme const& other) -> Scheme&;
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
    Uri(Uri&&) noexcept = default;
    Uri(Uri const&) = default;
    auto operator=(Uri&&) noexcept -> Uri& = default;
    auto operator=(Uri const&) -> Uri& = default;
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

inline auto operator<<(utils::StringStream& stream, http::Uri const& uri) -> utils::StringStream&
{
    switch (uri.get_target_form()) {
        case http::Uri::OriginForm: {
            auto const& pqf = uri.get_pqf();
            stream << *pqf->get_path();
            if (pqf->get_query().has_value()) {
                stream << "?" << *pqf->get_query();
            }
            if (pqf->get_fragment().has_value()) {
                stream << "#" << *pqf->get_fragment();
            }
        } break;
        case http::Uri::AbsoluteForm: {
            auto const& scheme = *uri.get_scheme();
            auto const& authority = *uri.get_authority();
            auto const& pqf = *uri.get_pqf();
            stream << scheme.get() << "://";
            if (authority.get_userinfo().has_value()) {
                stream << *authority.get_userinfo() << "@";
            }
            stream << authority.get_host();
            if (authority.get_port().has_value()) {
                stream << ":" << *authority.get_port();
            }
            stream << *pqf.get_path();
            if (pqf.get_query().has_value()) {
                stream << *pqf.get_query();
            }
            if (pqf.get_fragment().has_value()) {
                stream << *pqf.get_fragment();
            }
        } break;
        case http::Uri::AuthorityForm: {
            auto const& authority = *uri.get_authority();
            if (authority.get_userinfo().has_value()) {
                stream << *authority.get_userinfo() << "@";
            }
            stream << authority.get_host();
            if (authority.get_port().has_value()) {
                stream << ":" << *authority.get_port();
            }
        } break;
        case http::Uri::AsteriskForm: {
            stream << "*";
        } break;
        default: {
            throw std::logic_error("unreachable uri target form");
        } break;
    }
    return stream;
}

#endif //WEBSERV_HTTP_URI_HPP
