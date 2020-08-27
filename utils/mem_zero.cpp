//
// Created by boat on 8/26/20.
//

#include "mem_zero.hpp"

namespace utils {

void *ft_memset(void* string, int c, size_t length) {
    for (size_t idx = 0; idx < length; idx += 1) {
        reinterpret_cast<unsigned char*>(string)[idx] = static_cast<unsigned char>(c);
    }
    return string;
}

void ft_bzero(void* string, size_t length) {
    ft_memset(string, 0, length);
}

}