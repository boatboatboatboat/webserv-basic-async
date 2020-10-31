//
// Created by boat on 8/26/20.
//

#include <cstddef>

namespace utils {

auto ft_memcpy(void* dest, void const* src, size_t length) -> void*
{
    if (dest == src)
        return (dest);

    for (size_t idx = 0; idx < length; idx += 1) {
        reinterpret_cast<unsigned char*>(dest)[idx] = reinterpret_cast<unsigned char const*>(src)[idx];
    }
    return (dest);
}

auto ft_memmove(void* dest, const void* src, size_t length) -> void*
{
    if (length == 0 || dest == src) {
        return (dest);
    }
    size_t idx = (dest < src) ? 0 : length;
    if (dest < src) {
        ft_memcpy(dest, src, length);
    } else {
        while (idx > 0) {
            idx -= 1;
            ((unsigned char*)dest)[idx] = ((const unsigned char*)src)[idx];
        }
    }
    return (dest);
}
}
