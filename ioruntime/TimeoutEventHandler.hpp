//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
#define WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP

#include "ioruntime.hpp"

namespace ioruntime {

struct CallbackInfo {
    bool used;
    BoxFunctor functor;
};

class TimeoutEventHandler : public IEventHandler {
public:
    void reactor_step() override;
    void register_timeout(uint64_t ms, BoxFunctor&& callback);
private:
    static uint64_t get_time_ms();
    Mutex<std::multimap<uint64_t, CallbackInfo>> clocks_mutex;
};

}

#endif //WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
