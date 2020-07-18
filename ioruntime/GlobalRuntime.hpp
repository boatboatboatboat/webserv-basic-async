//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_HPP
#define WEBSERV_GLOBALRUNTIME_HPP

#include "../boxed/RcPtr.hpp"
#include "../mutex/mutex.hpp"
#include "ioruntime.hpp"

using boxed::RcPtr;
using mutex::Mutex;
using mutex::MutexGuard;

namespace ioruntime {
// Forward declarations
class Runtime;

// Class
class GlobalRuntime {
public:
    GlobalRuntime() = delete;
    ~GlobalRuntime() = delete;
    static MutexGuard<Runtime*>&& get();
    static void set_runtime(Runtime* new_runtime);

private:
    static Mutex<Runtime*> runtime;
};

} // namespace ioruntime

#endif // WEBSERV_GLOBALRUNTIME_HPP
