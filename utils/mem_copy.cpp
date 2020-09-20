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

}
