//
// Created by Djevayo Pattij on 7/18/20.
//

#ifndef WEBSERV_IORUNTIME_GLOBALIOEVENTHANDLER_HPP
#define WEBSERV_IORUNTIME_GLOBALIOEVENTHANDLER_HPP

#include "../func/Functor.hpp"
#include "../mutex/mutex.hpp"
#include "ioruntime.hpp"

namespace ioruntime {
using mutex::Mutex;
class IoEventHandler;

class GlobalIoEventHandler {
public:
    static void register_reader_callback(int fd, BoxFunctor&& x);

    static void register_writer_callback(int fd, BoxFunctor&& x);

    static void register_special_callback(int fd, BoxFunctor&& x);

    static void register_reader_callback_once(int fd, BoxFunctor&& x);
    static void register_writer_callback_once(int fd, BoxFunctor&& x);
    static void register_special_callback_once(int fd, BoxFunctor&& x);

    static void unregister_reader_callbacks(int fd);

    static void unregister_writer_callbacks(int fd);

    static void unregister_special_callbacks(int fd);

    static void set(IoEventHandler* handler);

private:
    static IoEventHandler* event_handler;
};
}

#endif //WEBSERV_IORUNTIME_GLOBALIOEVENTHANDLER_HPP
