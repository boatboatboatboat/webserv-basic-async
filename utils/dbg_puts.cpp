//
// Created by boat on 8/16/20.
//

#include <string>
#include <unistd.h>

#define DBG_OUT_FD STDERR_FILENO
void dbg_puts(std::string const& printme)
{
    unsigned int len = printme.length();
    const char* str = printme.c_str();
    int r = write(DBG_OUT_FD, str, len);
    unsigned int written = r;
    while (r >= 0 && (len - written)) {
        r = write(DBG_OUT_FD, str + written, len - written);
    }
}