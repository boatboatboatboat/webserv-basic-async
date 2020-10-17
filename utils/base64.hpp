//
// Created by boat on 17-10-20.
//

#ifndef WEBSERV_UTILS_BASE64_HPP
#define WEBSERV_UTILS_BASE64_HPP

#include <string>
#include <vector>
#include <algorithm>

namespace utils::base64 {

class Base64DecodeError: public std::runtime_error {
public:
    Base64DecodeError();
};

/*
auto decode_data(std::string_view str) -> std::vector<uint8_t>
{
}
auto encode_data(std::string_view str) -> std::vector<uint8_t>
{
}
*/

// Convert base64 string to internal string
// base64 -> string
[[nodiscard]] auto decode_string(std::string_view str) -> std::string;

// Convert internal string to base64 string
// string -> base64
[[nodiscard]] auto encode_string(std::string_view str) -> std::string;
}

#endif //WEBSERV_UTILS_BASE64_HPP
