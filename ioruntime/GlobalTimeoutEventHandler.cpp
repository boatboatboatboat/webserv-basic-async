//
// Created by Djevayo Pattij on 8/26/20.
//

#include "GlobalTimeoutEventHandler.hpp"

namespace ioruntime {

TimeoutEventHandler* GlobalTimeoutEventHandler::event_handler = nullptr;

void GlobalTimeoutEventHandler::set(TimeoutEventHandler* new_event_handler)
{
    event_handler = new_event_handler;
}

void GlobalTimeoutEventHandler::register_timeout(uint64_t ms, BoxFunctor&& callback)
{
    event_handler->register_timeout(ms, std::move(callback));
}

void GlobalTimeoutEventHandler::register_timeout_real(uint64_t ms, BoxFunctor&& callback)
{
    event_handler->register_timeout_real(ms, std::move(callback));
}

}