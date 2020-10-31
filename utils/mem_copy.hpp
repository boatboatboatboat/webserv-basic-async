//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_MEM_COPY_HPP
#define WEBSERV_MEM_COPY_HPP

#include <cstddef>
#include <cstring>

namespace utils {

void* ft_memcpy(void* dest, void const* src, size_t length);
auto ft_memmove(void* dest, const void* src, size_t length) -> void*;

template <typename T>
void memcpy(T& destination, T& source)
{
    if (&destination == &source)
        return;

    ft_memcpy(&destination, &source, sizeof(T));
}

template <typename T>
void memmove(T& destination, T& source)
{
    if (&destination == &source)
        return;

    ft_memmove(&destination, &source, sizeof(T));
}

} // namespace utils

#endif // WEBSERV_MEM_COPY_HPP
