//
// Created by boat on 10/28/20.
//

#ifndef WEBSERV_UTILS_MEMCOMPARE_HPP
#define WEBSERV_UTILS_MEMCOMPARE_HPP

#include <cstddef>

namespace utils {

inline int ft_memcmp(const void* str1, const void* str2, size_t length)
{
    const auto* cast_str1 = static_cast<const unsigned char*>(str1);
    const auto* cast_str2 = static_cast<const unsigned char*>(str2);
    size_t index = 0;
    while (index < length) {
        if (cast_str1[index] != cast_str2[index])
            return (cast_str1[index] - cast_str2[index]);
        index += 1;
    }
    return 0;
}

template <typename T>
int memcmp(T& a, T& b) {
    return ft_memcmp(&a, &b, sizeof(T));
}

}

#endif //WEBSERV_UTILS_MEMCOMPARE_HPP
