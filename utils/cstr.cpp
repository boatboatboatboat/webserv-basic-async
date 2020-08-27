//
// Created by boat on 8/26/20.
//

#include "cstr.hpp"

namespace utils {

size_t strlen(const char* s) {
    size_t l;
    for (l = 0; s[l]; l += 1)
        continue;
    return l;
}

}
