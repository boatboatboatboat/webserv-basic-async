//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP
#define WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP

#include "ioruntime.hpp"
#include "../option/optional.hpp"
#include <algorithm>
#include <csignal>
#include <map>

namespace ioruntime {

using std::multimap;
using option::optional;

class GlobalChildProcessHandler {
public:
    class ChildProcessHandler {
    public:
        ChildProcessHandler();
        void register_handler(pid_t process, BoxFunctor&& callback);

        Mutex<multimap<pid_t, BoxFunctor>> _process_handlers_mutex;
    };
    class SignalHandlerError : public std::runtime_error {
    public:
        SignalHandlerError();
    };
    static void register_handler(pid_t process, BoxFunctor&& callback);
    static auto get_cph() -> ChildProcessHandler&;
private:

    static optional<ChildProcessHandler> _cph;
};

}

#endif //WEBSERV_IORUNTIME_GLOBALCHILDPROCESSHANDLER_HPP
