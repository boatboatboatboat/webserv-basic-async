//
// Created by boat on 8/26/20.
//

#ifndef WEBSERV_CSTR_HPP
#define WEBSERV_CSTR_HPP

#include <cstddef>
#include <string>

namespace utils {
auto strlen(const char* s) -> size_t;
auto safe_streq(std::string_view l, std::string_view r) -> bool;
}

#endif //WEBSERV_CSTR_HPP
