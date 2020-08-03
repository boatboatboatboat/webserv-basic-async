//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
#define WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP

#include "ioruntime.hpp"

namespace ioruntime {
class TimeoutEventHandler : public IEventHandler {
public:
    void reactor_step() override;

private:
    static unsigned long get_time_ms();
    std::vector<unsigned long> clocks;
};
}

#endif //WEBSERV_IORUNTIME_TIMEOUTEVENTHANDLER_HPP
