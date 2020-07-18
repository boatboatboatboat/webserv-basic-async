//
// Created by boat on 11-07-20.
//

#ifndef WEBSERV_EVENTHANDLER_HPP
#define WEBSERV_EVENTHANDLER_HPP

namespace ioruntime {
class IEventHandler {
public:
    virtual ~IEventHandler() = 0;
    virtual void reactor_step() = 0;
};
} // namespace ioruntime

#endif // WEBSERV_EVENTHANDLER_HPP
