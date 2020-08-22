//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_MEM_COPY_HPP
#define WEBSERV_MEM_COPY_HPP

#include <cstddef>
#include <cstring>

namespace utils {
template <typename T>
void mem_copy(T& destination, T& source)
{
    if (&destination == &source)
        return;
    memcpy(&destination, &source, sizeof(T));
}
} // namespace utils

#endif // WEBSERV_MEM_COPY_HPP
