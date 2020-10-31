//
// Created by Djevayo Pattij on 9/6/20.
//

#include "Ipv6Address.hpp"
#include "../utils/MemCompare.hpp"
#include "../utils/StringStream.hpp"
#include "../utils/mem_zero.hpp"
#include "../utils/utils.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2

static auto local_inet_pton4(const char* src, const char* end, unsigned char* dst) -> int
{
    unsigned char tmp[NS_INADDRSZ];
    unsigned char* tp;

    int saw_digit = 0;
    int octets = 0;
    int ch;
    *(tp = tmp) = 0;

    while (src < end) {
        ch = *src++;
        if (ch >= '0' && ch <= '9') {
            unsigned int nu = *tp * 10 + (ch - '0');
            if (saw_digit && *tp == 0) {
                return 0;
            }
            if (nu > 255) {
                return 0;
            }
            *tp = nu;
            if (!saw_digit) {
                if (++octets > 4) {
                    return 0;
                }
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4) {
                return 0;
            }
            *++tp = 0;
            saw_digit = 0;
        } else {
            return 0;
        }
    }
    if (octets < 4) {
        return 0;
    }
    utils::ft_memcpy(dst, tmp, NS_INADDRSZ);
    return 1;
}

inline auto hex_digit_value(char ch) -> int
{
    if ('0' <= ch && ch <= '9') {
        return ch - '0';
    }
    if ('a' <= ch && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if ('A' <= ch && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

static auto local_inet_pton6(const char* src, const char* src_endp, unsigned char* dst) -> int
{
    unsigned char tmp[NS_IN6ADDRSZ];
    auto* tp = static_cast<unsigned char*>(utils::ft_memset(tmp, '\0', NS_IN6ADDRSZ));
    unsigned char* endp = tp + NS_IN6ADDRSZ;
    unsigned char* colonp = nullptr;

    if (src == src_endp) {
        return 0;
    }
    if (*src == ':') {
        ++src;
        if (src == src_endp || *src != ':') {
            return 0;
        }
    }

    const char* curtok = src;
    size_t xdigits_seen = 0;
    unsigned int val = 0;
    int ch;

    while (src < src_endp) {
        ch = *src++;
        int digit = hex_digit_value(ch);
        if (digit >= 0) {
            if (xdigits_seen == 4) {
                return 0;
            }
            val <<= 4;
            val |= digit;
            if (val > 0xffff) {
                return 0;
            }
            ++xdigits_seen;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (xdigits_seen == 0) {
                if (colonp) {
                    return 0;
                }
                colonp = tp;
                continue;
            } else if (src == src_endp) {
                return 0;
            }
            if (tp + NS_INT16SZ > endp) {
                return 0;
            }
            *tp++ = (unsigned char)(val >> 8) & 0xff;
            *tp++ = (unsigned char)val & 0xff;
            xdigits_seen = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) && local_inet_pton4(curtok, src_endp, tp) > 0) {
            tp += NS_INADDRSZ;
            xdigits_seen = 0;
            break;
        }
        return 0;
    }
    if (xdigits_seen > 0) {
        if (tp + NS_INT16SZ > endp) {
            return 0;
        }
        *tp++ = (unsigned char)(val >> 8) & 0xff;
        *tp++ = (unsigned char)val & 0xff;
    }
    if (colonp != nullptr) {
        if (tp == endp) {
            return 0;
        }
        size_t n = tp - colonp;
        utils::ft_memmove(endp - n, colonp, n);
        utils::ft_memset(colonp, 0, endp - n - colonp);
        tp = endp;
    }
    if (tp != endp) {
        return 0;
    }
    utils::ft_memcpy(dst, tmp, NS_IN6ADDRSZ);
    return 1;
}

auto net::Ipv6Address::get_ip_posix() const -> in6_addr
{
    return ip;
}

net::Ipv6Address::Ipv6Address(uint64_t b1, uint64_t b2)
{
    auto* x = reinterpret_cast<uint64_t*>(&ip);
    x[0] = b1;
    x[1] = b2;
}

net::Ipv6Address::Ipv6Address(string_view str)
{
    std::string x(str);
    int r = local_inet_pton6(x.c_str(), x.c_str() + x.size(), reinterpret_cast<unsigned char*>(&ip));
    if (r != 1) {
        throw std::runtime_error("bad ip address");
    }
}

net::Ipv6Address::Ipv6Address(in6_addr addr)
    : ip(addr)
{
}

auto net::Ipv6Address::operator==(const net::Ipv6Address& rhs) const -> bool
{
    return utils::memcmp(ip, rhs.ip) == 0;
}

auto net::Ipv6Address::operator!=(const net::Ipv6Address& rhs) const -> bool
{
    return utils::memcmp(ip, rhs.ip) != 0;
}

auto net::Ipv6Address::operator>(const net::Ipv6Address& rhs) const -> bool
{
    return utils::memcmp(ip, rhs.ip) > 0;
}

auto net::Ipv6Address::operator<(const net::Ipv6Address& rhs) const -> bool
{
    return utils::memcmp(ip, rhs.ip) < 0;
}

auto operator<<(utils::StringStream& os, const net::Ipv6Address& sa) -> utils::StringStream&
{
    in6_addr ip_raw = sa.get_ip_posix();
    auto* ip = reinterpret_cast<uint16_t*>(&ip_raw);
    size_t idx = 0;
    bool pdouble = false;
    while (idx < 8) {
        if (ip[idx] == 0 && idx != 7 && ip[idx + 1] == 0) {
            while (ip[idx] == 0 && idx < 8)
                idx += 1;
            if (pdouble)
                os << ":";
            else
                os << "::";
            pdouble = false;
            continue;
        } else {
            os << utils::uint64_to_hexstring(__builtin_bswap16(ip[idx]));
        }
        if (idx < 7) {
            pdouble = true;
            os << ":";
        }
        idx += 1;
    }
    return os;
}