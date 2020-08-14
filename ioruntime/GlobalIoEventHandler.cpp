//
// Created by Djevayo Pattij on 7/18/20.
//

#include "GlobalIoEventHandler.hpp"

namespace ioruntime {
IoEventHandler* GlobalIoEventHandler::event_handler = nullptr;

void GlobalIoEventHandler::register_reader_callback(int fd, BoxFunctor&& x)
{
    event_handler->register_reader_callback(fd, std::move(x));
}

void GlobalIoEventHandler::register_writer_callback(int fd, BoxFunctor&& x)
{
    event_handler->register_writer_callback(fd, std::move(x));
}

void GlobalIoEventHandler::register_special_callback(int fd, BoxFunctor&&x)
{
    event_handler->register_special_callback(fd, std::move(x));
}

void GlobalIoEventHandler::unregister_reader_callbacks(int fd)
{
    event_handler->unregister_reader_callbacks(fd);
}

void GlobalIoEventHandler::unregister_writer_callbacks(int fd)
{
    event_handler->unregister_writer_callbacks(fd);
}

void GlobalIoEventHandler::unregister_special_callbacks(int fd)
{
    event_handler->unregister_special_callbacks(fd);
}

void GlobalIoEventHandler::set(IoEventHandler* handler)
{
    event_handler = handler;
}

void GlobalIoEventHandler::register_reader_callback_once(int fd, BoxFunctor &&x)
{
    event_handler->register_reader_callback_once(fd, std::move(x));
}

void GlobalIoEventHandler::register_writer_callback_once(int fd, BoxFunctor &&x)
{
    event_handler->register_writer_callback_once(fd, std::move(x));
}

void GlobalIoEventHandler::register_special_callback_once(int fd, BoxFunctor &&x)
{
    event_handler->register_special_callback_once(fd, std::move(x));
}
}