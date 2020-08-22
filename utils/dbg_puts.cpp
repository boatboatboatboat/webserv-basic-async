//
// Created by boat on 8/16/20.
//

#include <string>
#include <unistd.h>

void dbg_puts(std::string const& printme)
{
    unsigned int len = printme.length();
    const char* str = printme.c_str();
    int r = write(1, str, len);
    unsigned int written = r;
    while (r >= 0 && (len - written)) {
        r = write(1, str + written, len - written);
    }
}