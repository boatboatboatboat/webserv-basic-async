//
// Created by Djevayo Pattij on 8/3/20.
//

#include "TimeoutEventHandler.hpp"
#include <sys/time.h>

namespace ioruntime {
unsigned long
TimeoutEventHandler::get_time_ms()
{
    timeval tv { 0, 0 };

    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
}
