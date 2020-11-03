//
// Created by boat on 8/26/20.
//

#include "cstr.hpp"

namespace utils {

auto strlen(const char* s) -> size_t
{
    size_t l;
    for (l = 0; s[l]; l += 1)
        continue;
    return l;
}

auto safe_streq(std::string_view unsafe, std::string_view safe) -> bool {
    size_t usl = unsafe.length();
    size_t sl = safe.length();

    volatile size_t i = 0;
    volatile size_t j = 0;
    volatile size_t k = 0;

    int eq = 0;

    while (i < usl) {
        eq |= unsafe[i] ^ safe[j];

        if (i >= sl) {
            k = k + 1;
        } else {
            j = j + 1;
        }
        i = i + 1;
    }

    volatile bool x = usl == sl;
    x = x & (eq == 0);
    return x;
}

}
