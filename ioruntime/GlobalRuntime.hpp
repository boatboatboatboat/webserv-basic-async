//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_HPP
#define WEBSERV_GLOBALRUNTIME_HPP

#include "ioruntime.hpp"
#include "../mutex/mutex.hpp"
#include "../boxed/RcPtr.hpp"

using mutex::Mutex;
using boxed::RcPtr;

namespace ioruntime {
    class GlobalRuntime {
    public:
        Mutex<RcPtr<Runtime>>& get();
    private:
        // fixme: this should be a weak ptr?
        Mutex<RcPtr<Runtime>> runtime;
    };
}

#endif //WEBSERV_GLOBALRUNTIME_HPP
