//
// Created by boat on 8/26/20.
//

#ifndef WEBSERV_MEM_ZERO_HPP
#define WEBSERV_MEM_ZERO_HPP

#include <cstddef>

namespace utils {

void ft_bzero(void* string, size_t length);
void *ft_memset(void* string, int c, size_t length);

template<typename T>
void memset(T& t, int c)
{
    ft_memset(&t, c, sizeof(T));
}

template<typename T>
void bzero(T& t)
{
    ft_bzero(&t, sizeof(T));
}

}

#endif //WEBSERV_MEM_ZERO_HPP
