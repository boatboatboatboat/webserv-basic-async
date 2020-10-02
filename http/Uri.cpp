//
// Created by boat on 9/26/20.
//

#include "Uri.hpp"
#include "HttpRfcConstants.hpp"

namespace http {

Scheme::SchemeParseError::SchemeParseError()
    : std::runtime_error("URI scheme parsing failed")
{
}

Scheme::Scheme()
    : _type(None)
{
}

Scheme::Scheme(string_view view)
{
    _type = Standard;
    if (view == "http") {
        new (&_standard) StandardScheme("http");
    } else if (view == "https") {
        new (&_standard) StandardScheme("https");
    } else {
        using namespace parser_utils;

        if (view.empty()) {
            throw SchemeParseError();
        }
        if (!is_alpha(view[0])) {
            throw SchemeParseError();
        }
        if (view.length() > 1) {
            for (auto c : view.substr(1, view.length())) {
                if (!is_alpha(c) && !is_digit(c) && c != '=' && c != '-' && c != '.') {
                    throw SchemeParseError();
                }
            }
            new (&_custom) CustomScheme(view);
        }
    }
}

auto Scheme::operator=(Scheme&& other) noexcept -> Scheme&
{
    switch (_type) {
    case None: {
    } break;
    case Standard: {
        _standard.~StandardScheme();
    } break;
    case Custom: {
        _custom.~CustomScheme();
    } break;
    }
    _type = other._type;
    switch (_type) {
    case None: {
    } break;
    case Standard: {
        _standard = other._standard;
    } break;
    case Custom: {
        _custom = std::move(other._custom);
    } break;
    }
    return *this;
}

Scheme::Scheme(Scheme&& other) noexcept
{
    switch (other._type) {
    case None: {
    } break;
    case Standard: {
        new (&_standard) StandardScheme(other._standard);
    } break;
    case Custom: {
        new (&_custom) CustomScheme(std::move(other._custom));
    } break;
    }
    _type = other._type;
}

Scheme::~Scheme()
{
    switch (_type) {
    case Standard: {
        _standard.~StandardScheme();
    } break;
    case Custom: {
        _custom.~CustomScheme();
    } break;
    default:
        break;
    }
}

auto Scheme::is_standard() const -> bool
{
    return _type == Standard;
}

auto Scheme::is_none() const -> bool
{
    return _type == None;
}

auto Scheme::is_custom() const -> bool
{
    return _type == Custom;
}

auto Scheme::get() const -> string_view
{
    switch (_type) {
    case None: {
        throw std::runtime_error("bad get");
    } break;
    case Standard: {
        return string_view(_standard);
    } break;
    case Custom: {
        return string_view(_custom);
    } break;
    }
}

Scheme::Scheme(const Scheme& other)
{
    switch (other._type) {
    case None: {
    } break;
    case Standard: {
        new (&_standard) StandardScheme(other._standard);
    } break;
    case Custom: {
        new (&_custom) CustomScheme(other._custom);
    } break;
    }
    _type = other._type;
}

auto Scheme::operator=(const Scheme& other) -> Scheme&
{
    if (_type == Custom) {
        _custom.~CustomScheme();
    }
    switch (other._type) {
    case None: {
    } break;
    case Standard: {
        new (&_standard) StandardScheme(other._standard);
    } break;
    case Custom: {
        new (&_custom) CustomScheme(other._custom);
    } break;
    }
    _type = other._type;
    return *this;
}

Uri::UriParseError::UriParseError()
    : std::runtime_error("URI parse failed")
{
}

Authority::Authority(string_view str)
{
    using namespace parser_utils;

    // authority = [ userinfo "@" ] host [ ":" port ]
    // so if there's an @, there's a userinfo,
    // and if there's a :, there's a port
    //
    // note:
    // the username:password format is deprecated
    auto userinfo_end = str.find('@');

    if (userinfo_end != string_view::npos) {
        // we have userinfo, parse the userinfo
        size_t idx = 0;
        string userinfo;

        while (idx < userinfo_end) {
            auto c = str[idx];
            if (is_unreserved(c)) {
                userinfo += c;
                idx += 1;
                continue;
            }
            if (is_pct_encoded(str.substr(idx, 3))) {
                userinfo += str.substr(idx, 3);
                idx += 3;
                continue;
            }
            if (is_sub_delims(c) || c == ':') {
                userinfo += c;
                idx += 1;
                continue;
            }
            throw AuthorityParseError();
        }

        _userinfo = std::move(userinfo);
    }

    auto port_start = str.find(':', userinfo_end == string_view::npos ? 0 : userinfo_end);

    // FIXME: read below
    // an empty port is legal, but it should be treated as if there were no port
    if (port_start != string_view::npos) {
        size_t idx = port_start + 1;
        uint32_t port = 0;

        if (idx != str.length()) {
            while (idx < str.length()) {
                auto c = str[idx];
                if (!is_digit(c))
                    throw AuthorityParseError();
                port *= 10;
                port += c - '0';
                if (port > UINT16_MAX)
                    throw AuthorityParseError();
                idx += 1;
            }

            _port = port & 0xFFFFu;
        }
    }

    auto host_start = userinfo_end == string_view::npos ? 0 : userinfo_end + 1;
    auto host_len = port_start == string_view::npos ? str.length() - host_start : port_start - host_start;

    auto host = str.substr(host_start, host_len);
    {
        size_t idx = 0;

        while (idx < host.length()) {
            auto c = host[idx];
            if (is_unreserved(c)) {
                idx += 1;
                continue;
            }
            if (is_pct_encoded(host.substr(idx, 3))) {
                idx += 3;
                continue;
            }
            if (is_sub_delims(c)) {
                idx += 1;
                continue;
            }
            throw AuthorityParseError();
        }
    }
    _host = string(host);
    /*
*/
}

auto Authority::get_userinfo() const -> optional<string> const&
{
    return _userinfo;
}

auto Authority::get_host() const -> string const&
{
    return _host;
}

auto Authority::get_port() const -> optional<uint16_t>
{
    return optional<uint16_t>(_port);
}

Authority::AuthorityParseError::AuthorityParseError()
    : std::runtime_error("URI authority parse failed")
{
}

PathQueryFragment::PathQueryFragmentError::PathQueryFragmentError()
    : std::runtime_error("URI path/query/fragment parse failed")
{
}

PathQueryFragment::PathQueryFragment(string_view str)
{
    // empty pqf is valid in the context of an absolute-URI
    if (str.empty()) {
        // empty pqf
        return;
    }

    // a non-empty pqf should start with /, ? or #
    if (!str.starts_with('/') && !str.starts_with('?') && !str.starts_with('#')) {
        throw PathQueryFragmentError();
    }

    using namespace parser_utils;

    if (str.starts_with('/')) {
        str.remove_prefix(1);

        string path_escaped = "/";
        string path;

        while (!str.empty()) {
            auto c = str.front();
            if (is_unreserved(c)) {
                path_escaped += c;
                if (!path.empty()) {
                    path += c;
                }
                str.remove_prefix(1);
                continue;
            }
            if (is_pct_encoded(str.substr(0, 3))) {
                if (path.empty())
                    path += path_escaped;
                path_escaped += pct_decode(str.substr(0, 3));
                path += str.substr(0, 3);
                str.remove_prefix(3);
                continue;
            }
            if (is_sub_delims(c) || c == ':' || c == '@' || c == '/') {
                path_escaped += c;
                if (!path.empty()) {
                    path += c;
                }
                str.remove_prefix(1);
                continue;
            }
            break;
        }

        if (path.empty()) {
            _path = Path { path_escaped, option::nullopt };
        } else {
            _path = Path { path_escaped, path };
        }
    }

    if (str.empty()) {
        return;
    }

    if (str.front() != '?' && str.front() != '#')
        throw PathQueryFragmentError();

    if (str.front() == '?') {
        str.remove_prefix(1);

        string query;

        while (!str.empty()) {
            auto c = str.front();
            if (is_unreserved(c)) {
                query += c;
                str.remove_prefix(1);
                continue;
            }
            if (is_pct_encoded(str.substr(0, 3))) {
                query += str.substr(0, 3);
                str.remove_prefix(3);
                continue;
            }
            if (is_sub_delims(c) || c == ':' || c == '@' || c == '/' || c == '?') {
                query += c;
                str.remove_prefix(1);
                continue;
            }
            break;
        }

        _query = std::move(query);
    }

    if (str.empty()) {
        return;
    } else if (str.front() == '#') {
        str.remove_prefix(1);
        string fragment;

        while (!str.empty()) {
            auto c = str.front();
            if (is_unreserved(c)) {
                fragment += c;
                str.remove_prefix(1);
                continue;
            }
            if (is_pct_encoded(str.substr(0, 3))) {
                fragment += str.substr(0, 3);
                str.remove_prefix(3);
                continue;
            }
            if (is_sub_delims(c) || c == ':' || c == '@' || c == '/' || c == '?') {
                fragment += c;
                str.remove_prefix(1);
                continue;
            }
            throw PathQueryFragmentError();
        }

        _fragment = std::move(fragment);
    } else {
        throw PathQueryFragmentError();
    }
}

auto PathQueryFragment::get_path() const -> optional<string_view>
{
    if (_path.has_value()) {
        if (_path->_unescaped.has_value()) {
            return optional<string_view>(*_path->_unescaped);
        } else {
            return optional<string_view>(_path->_escaped);
        }
    } else {
        return option::nullopt;
    }
}

auto PathQueryFragment::get_path_escaped() const -> optional<string_view>
{
    if (_path.has_value()) {
        return optional<string_view>(_path->_escaped);
    } else {
        return option::nullopt;
    }
}

auto PathQueryFragment::get_query() const -> optional<string_view>
{
    return _query.has_value() ? optional<string_view>(_query) : option::nullopt;
}

auto PathQueryFragment::get_fragment() const -> optional<string_view>
{
    return _fragment.has_value() ? optional<string_view>(_fragment) : option::nullopt;
}

Uri::Uri(string_view str)
{
    if (str == "*") {
        _target_form = AsteriskForm;
    } else if (str.starts_with('/')) {
        _target_form = OriginForm;
        _pqf = PathQueryFragment(str);
    } else {
        auto scheme_end = str.find("://");
        if (scheme_end == string_view::npos) {
            _target_form = AuthorityForm;
            _authority = Authority(str);
        } else {
            _target_form = AbsoluteForm;
            _scheme = Scheme(str.substr(0, scheme_end));
            auto authority_start = scheme_end + 3;
            auto authority_end = str.find('/', authority_start);
            if (authority_end == string_view::npos) {
                authority_end = str.find('?', authority_start);
                if (authority_end == string_view::npos) {
                    authority_end = str.find('#', authority_start);
                    if (authority_end == string_view::npos) {
                        authority_end = str.length();
                    }
                }
            }
            auto test = str.substr(authority_start, authority_end - authority_start);
            _authority = Authority(test);
            _pqf = PathQueryFragment(str.substr(authority_end, str.length()));
        }
    }
}

auto Uri::get_target_form() const -> Uri::RequestTargetForm
{
    return _target_form;
}

auto Uri::get_scheme() const -> optional<Scheme> const&
{
    return _scheme;
}

auto Uri::get_authority() const -> optional<Authority> const&
{
    return _authority;
}

auto Uri::get_pqf() const -> optional<PathQueryFragment> const&
{
    return _pqf;
}

}