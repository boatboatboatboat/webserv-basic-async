//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_MEM_COPY_HPP
#define WEBSERV_MEM_COPY_HPP

#include <cstddef>
#include <cstring>

namespace utils {

void* ft_memcpy(void* dest, void const* src, size_t length);

template <typename T>
void memcpy(T& destination, T& source)
{
    if (&destination == &source)
        return;

    ft_memcpy(&destination, &source, sizeof(T));
}

} // namespace utils

#endif // WEBSERV_MEM_COPY_HPP
