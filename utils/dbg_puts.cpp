//
// Created by boat on 8/16/20.
//

#include <string>
#include <unistd.h>

void dbg_puts(std::string const& printme)
{
    unsigned int len = printme.length();
    const char* str = printme.c_str();
    int r = write(STDOUT_FILENO, str, len);
    unsigned int written = r;
    while (r >= 0 && (len - written)) {
        r = write(STDOUT_FILENO, str + written, len - written);
    }
}