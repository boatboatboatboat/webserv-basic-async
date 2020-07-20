//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_HPP
#define WEBSERV_GLOBALRUNTIME_HPP

#include "../boxed/RcPtr.hpp"
#include "../mutex/mutex.hpp"
#include "ioruntime.hpp"
#include "../futures/futures.hpp"

using boxed::RcPtr;
using mutex::Mutex;
using mutex::MutexGuard;
using futures::IFuture;

namespace ioruntime {
// Forward declarations
class Runtime;

// Class
class GlobalRuntime {
public:
    GlobalRuntime() = delete;
    ~GlobalRuntime() = delete;
    static MutexGuard<Runtime*> get();
    static void set_runtime(Runtime* new_runtime);
    static void spawn(BoxPtr<IFuture<void>>&& fut);

private:
    static Mutex<Runtime*> runtime;
};

} // namespace ioruntime

#endif // WEBSERV_GLOBALRUNTIME_HPP
