//
// Created by boat on 9/28/20.
//

#include "GlobalChildProcessHandler.hpp"

#ifdef __linux__
#include <wait.h>
#elif __APPLE__
#include <sys/wait.h>
#endif

namespace ioruntime {

GlobalChildProcessHandler::ChildProcessHandler::ChildProcessHandler()
{
    auto res = signal(SIGCHLD, [](int _) {
        (void)_;
        auto lock = GlobalChildProcessHandler::get_cph()._some_pid_is_ready_lol.lock();
        *lock += 1;
    });

    if (res == SIG_ERR) {
        throw SignalHandlerError();
    }
}

auto GlobalChildProcessHandler::get_cph() -> GlobalChildProcessHandler::ChildProcessHandler&
{
    // lazily initialize the process handler
    // this will also provide error throwing in non-global-ctor code
    if (!_cph.has_value()) {
        _cph = ChildProcessHandler();
    }
    return _cph.value();
}

void GlobalChildProcessHandler::ChildProcessHandler::register_handler(pid_t process, BoxFunctor&& callback)
{
    auto process_handlers = _process_handlers_mutex.lock();
    process_handlers->emplace(process, std::move(callback));
}

void GlobalChildProcessHandler::ChildProcessHandler::reactor_step()
{
    size_t is_ready;
    {
        auto lock = _some_pid_is_ready_lol.lock();
        is_ready = *lock;
    }
    if (is_ready) {
        for (;;) {
            int wstatus;

            // WNOHANG is set because otherwise we'll have a hanging interrupt
            // also if SIGCHLD is fired, there *should* be a process ready,
            auto pid = waitpid(0, &wstatus, WNOHANG);

            if (pid > 0) {
                std::vector<BoxFunctor> callbacks;
                {
                    auto processes = GlobalChildProcessHandler::get_cph()
                                         ._process_handlers_mutex
                                         .lock();

                    auto range = processes->equal_range(pid);
                    auto it = range.first;
                    while (it != range.second) {
                        auto& pcb = *it;
                        callbacks.push_back(std::move(pcb.second));
                        it = processes->erase(it);
                    }
                }
                if (WIFEXITED(wstatus)) {
                    for (auto& cb : callbacks) {
                        (*cb)();
                    }
                }
                is_ready -= 1;
            } else {
                break;
            }
        }
        {
            auto lock = _some_pid_is_ready_lol.lock();
            *lock = is_ready;
        }
    }
}

void GlobalChildProcessHandler::register_handler(pid_t process, BoxFunctor&& callback)
{
    get_cph().register_handler(process, std::move(callback));
}

optional<GlobalChildProcessHandler::ChildProcessHandler> GlobalChildProcessHandler::_cph = option::nullopt;

void GlobalChildProcessHandler::reactor_step()
{
    get_cph().reactor_step();
}

GlobalChildProcessHandler::SignalHandlerError::SignalHandlerError()
    : std::runtime_error("Signal handler error")
{
}

}