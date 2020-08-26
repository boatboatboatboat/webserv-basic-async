//
// Created by boat on 8/16/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_IPP
#define WEBSERV_GLOBALRUNTIME_IPP

namespace ioruntime {

template <typename T>
void GlobalRuntime::spawn(T&& fut)
{
    GlobalRuntime::spawn_boxed(BoxPtr<T>(std::forward<T>(fut)));
}

}

#endif //WEBSERV_GLOBALRUNTIME_IPP
