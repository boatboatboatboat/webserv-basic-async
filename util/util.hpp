//
// Created by boat on 8/17/20.
//

#ifndef WEBSERV_UTIL_HPP
#define WEBSERV_UTIL_HPP
#include "mem_copy.hpp"
#include <string>

void dbg_puts(std::string const& printme);

#ifdef __linux__

// gettid() only works on linux

#define DBGPRINT(x)                      \
    do {                                 \
        std::string __x(__func__);       \
        __x += (" ");                    \
        __x.append(__FILE__);            \
        __x.append(":");                 \
        __x += std::to_string(__LINE__); \
        __x.append(" ");                 \
        __x += std::to_string(gettid()); \
        __x.append(" ");                 \
        __x.append(x);                   \
        __x.append("\n");                \
        dbg_puts(__x);                   \
    } while (0)

#endif
#ifdef __APPLE__

#define DBGPRINT(x)                      \
    do {                                 \
        std::string __x(__func__);       \
        __x += (" ");                    \
        __x.append(__FILE__);            \
        __x.append(":");                 \
        __x += std::to_string(__LINE__); \
        __x.append(" ");                 \
        __x.append(x);                   \
        __x.append("\n");                \
        dbg_puts(__x);                   \
    } while (0)

#endif

#endif //WEBSERV_UTIL_HPP
