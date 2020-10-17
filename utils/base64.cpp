//
// Created by boat on 17-10-20.
//

#include "base64.hpp"
#include "../option/optional.hpp"

using option::optional;

namespace utils::base64 {
inline auto to_radix(uint8_t c) -> char
{
    constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "0123456789"
                                       "+/";
    return chars[c];
}

inline auto from_radix(char c) -> uint8_t
{
    constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "0123456789"
                                       "+/";

    auto f = chars.find(c);
    if (f == std::string_view::npos) {
        throw Base64DecodeError();
    } else {
        return f;
    }
}

auto decode_string(std::string_view str) -> std::string
{
    std::string out;

    while (str.length() >= 4) {
        uint8_t group[4] = { 0 };
        group[0] = from_radix(str[0]);
        group[1] = from_radix(str[1]);
        out += (group[0] << 2) | ((group[1] & 0b00110000) >> 4);
        if (str[2] != '=') {
            group[2] = from_radix(str[2]);
            out += ((group[1] & 0b00001111) << 4) | ((group[2] & 0b00111100) >> 2);
            if (str[3] != '=') {
                group[3] = from_radix(str[3]);
                out += ((group[2] & 0b00000011) << 6) | group[3];
            }
        }
        str.remove_prefix(4);
    }
    if (!str.empty()) {
        throw Base64DecodeError();
    }
    return out;
}

auto encode_string(std::string_view str) -> std::string
{
    std::string out;

    while (!str.empty()) {
        uint8_t group[4] = { 0 };
        if (str.length() == 2) {
            std::string_view cs = str.substr(0, 3);
            group[0] = (cs[0] & 0b11111100) >> 2;
            group[1] = (cs[0] & 0b00000011) << 4;
            group[1] |= (cs[1] & 0b11110000) >> 4;
            group[2] = (cs[1] & 0b00001111) << 2;
            group[2] |= (cs[2] & 0b11000000) >> 6;
            str.remove_prefix(2);
            out += to_radix(group[0]);
            out += to_radix(group[1]);
            out += to_radix(group[2]);
            out += '=';
        } else if (str.length() == 1) {
            std::string_view cs = str.substr(0, 3);
            group[0] = (cs[0] & 0b11111100) >> 2;
            group[1] = (cs[0] & 0b00000011) << 4;
            group[1] |= (cs[1] & 0b11110000) >> 4;
            out += to_radix(group[0]);
            out += to_radix(group[1]);
            out += '=';
            out += '=';
            str.remove_prefix(1);
        } else {
            std::string_view cs = str.substr(0, 3);
            group[0] = (cs[0] & 0b11111100) >> 2;
            group[1] = (cs[0] & 0b00000011) << 4;
            group[1] |= (cs[1] & 0b11110000) >> 4;
            group[2] = (cs[1] & 0b00001111) << 2;
            group[2] |= (cs[2] & 0b11000000) >> 6;
            group[3] = (cs[2] & 0b00111111);
            str.remove_prefix(3);
            out += to_radix(group[0]);
            out += to_radix(group[1]);
            out += to_radix(group[2]);
            out += to_radix(group[3]);
        }
    }
    return out;
}

Base64DecodeError::Base64DecodeError()
    : std::runtime_error("base64 decode failed")
{
}
}