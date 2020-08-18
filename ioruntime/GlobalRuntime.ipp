//
// Created by boat on 8/16/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_IPP
#define WEBSERV_GLOBALRUNTIME_IPP

#include "GlobalRuntime.hpp"

namespace ioruntime {
template <typename T>
void GlobalRuntime::spawn_r(T&& fut)
{
    /*    auto mut = GlobalRuntime::get();
    auto lock = mut->lock();
    (*lock)->spawn(BoxPtr<T>(std::forward<T>(fut)));
*/
}
}

#endif //WEBSERV_GLOBALRUNTIME_IPP
