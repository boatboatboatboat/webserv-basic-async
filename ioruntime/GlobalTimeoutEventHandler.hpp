//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_IORUNTIME_GLOBALTIMEOUTEVENTHANDLER_HPP
#define WEBSERV_IORUNTIME_GLOBALTIMEOUTEVENTHANDLER_HPP

#include "TimeoutEventHandler.hpp"

namespace ioruntime {

class GlobalTimeoutEventHandler {
public:
    static void set(TimeoutEventHandler* new_event_handler);
    static auto register_timeout(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection;
    static auto register_timeout_real(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection;
    static bool is_clocks_empty();

private:
    static TimeoutEventHandler* event_handler;
};

}

#endif //WEBSERV_IORUNTIME_GLOBALTIMEOUTEVENTHANDLER_HPP
