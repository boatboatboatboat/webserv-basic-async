//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
#define WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP

#include "ioruntime.hpp"

namespace ioruntime {

struct CallbackInfo {
    RcPtr<Mutex<bool>> disconnected;
    BoxFunctor functor;
    bool used;
};

class TimeoutEventConnection {
public:
    TimeoutEventConnection() = delete;
    TimeoutEventConnection(TimeoutEventConnection const&) = delete;
    TimeoutEventConnection& operator=(TimeoutEventConnection const&) = delete;
    TimeoutEventConnection(TimeoutEventConnection&&) noexcept;
    TimeoutEventConnection& operator=(TimeoutEventConnection&&) noexcept;
    explicit TimeoutEventConnection(RcPtr<Mutex<bool>>&& handle);
    ~TimeoutEventConnection();
    void disconnect();
private:
    bool _disconnected = false;
    RcPtr<Mutex<bool>> _handle;
};

class TimeoutEventHandler : public IEventHandler {
public:
    void reactor_step() override;
    TimeoutEventConnection register_timeout_real(uint64_t ms, BoxFunctor&& callback);
    TimeoutEventConnection register_timeout(uint64_t ms, BoxFunctor&& callback);
    static uint64_t get_time_ms();
    bool is_clocks_empty();

private:
    Mutex<std::multimap<uint64_t, CallbackInfo>> clocks_mutex;
};

}

#endif //WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
