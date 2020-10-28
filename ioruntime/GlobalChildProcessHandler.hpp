//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP
#define WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP

#include "../option/optional.hpp"
#include "ioruntime.hpp"
#include <algorithm>
#include <csignal>
#include <map>

namespace ioruntime {

using option::optional;
using std::map;

class GlobalChildProcessHandler: public IEventHandler {
public:
    class ChildProcessHandler : public IEventHandler {
    public:
        ChildProcessHandler();
        ChildProcessHandler(ChildProcessHandler&&) noexcept = default;
        auto operator=(ChildProcessHandler&&) noexcept -> ChildProcessHandler& = default;
        ChildProcessHandler(ChildProcessHandler const&) = delete;
        auto operator=(ChildProcessHandler const&) -> ChildProcessHandler& = delete;
        ~ChildProcessHandler() override = default;

        void register_handler(pid_t process, BoxFunctor&& callback);
        void reactor_step() override;

        Mutex<size_t> _some_pid_is_ready_lol;
        Mutex<map<pid_t, BoxFunctor>> _process_handlers_mutex;
    };
    class SignalHandlerError : public std::runtime_error {
    public:
        SignalHandlerError();
    };
    static void register_handler(pid_t process, BoxFunctor&& callback);
    static auto get_cph() -> ChildProcessHandler&;
    void reactor_step() override;

private:
    static optional<ChildProcessHandler> _cph;
};

}

#endif //WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP
