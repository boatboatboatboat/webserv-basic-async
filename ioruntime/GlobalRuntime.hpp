//
// Created by boat on 7/17/20.
//

#ifndef WEBSERV_GLOBALRUNTIME_HPP
#define WEBSERV_GLOBALRUNTIME_HPP

#include "../boxed/BoxPtr.hpp"
#include "../futures/futures.hpp"
#include "../mutex/mutex.hpp"
#include "ioruntime.hpp"

using boxed::BoxPtr;
using futures::IFuture;
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
    static Mutex<Runtime*>* get();
    static void set_runtime(Runtime* new_runtime);
    static void spawn_boxed(BoxPtr<IFuture<void>>&& fut);

    template <typename T>
    static void spawn(T&& fut);

private:
    static Mutex<Runtime*> runtime;
};

} // namespace ioruntime

#include "GlobalRuntime.ipp"

#endif // WEBSERV_GLOBALRUNTIME_HPP
