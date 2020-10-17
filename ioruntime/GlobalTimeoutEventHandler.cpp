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

auto GlobalTimeoutEventHandler::register_timeout(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection
{
    return event_handler->register_timeout(ms, std::move(callback));
}

auto GlobalTimeoutEventHandler::register_timeout_real(uint64_t ms, BoxFunctor&& callback) -> TimeoutEventConnection
{
    return event_handler->register_timeout_real(ms, std::move(callback));
}

bool GlobalTimeoutEventHandler::is_clocks_empty()
{
    return event_handler->is_clocks_empty();
}

}