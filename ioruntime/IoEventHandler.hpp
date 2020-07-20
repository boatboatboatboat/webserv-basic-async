//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_IOEVENTHANDLER_HPP
#define WEBSERV_IOEVENTHANDLER_HPP

#include <map>
#include <zconf.h>

#include "../boxed/BoxPtr.hpp"
#include "../func/Functor.hpp"
#include "../futures/futures.hpp"
#include "IEventHandler.hpp"

using futures::Waker;
using ioruntime::IEventHandler;

namespace ioruntime {
class IoEventHandler : public IEventHandler {
    struct CallbackInfo {
        bool once;
        BoxFunctor bf;
    };
public:
    IoEventHandler();
    void register_reader_callback(int fd, BoxFunctor&& x);
    void register_writer_callback(int fd, BoxFunctor&& x);
    void register_special_callback(int fd, BoxFunctor&& x);
    void register_reader_callback_once(int fd, BoxFunctor&& x);
    void register_writer_callback_once(int fd, BoxFunctor&& x);
    void register_special_callback_once(int fd, BoxFunctor&& x);
    void unregister_reader_callbacks(int fd);
    void unregister_writer_callbacks(int fd);
    void unregister_special_callbacks(int fd);
    void reactor_step() override;

private:
    static void fire_listeners_for(int fd, fd_set& selected, std::multimap<int, CallbackInfo>& listeners);

    fd_set readfds;
    fd_set writefds;
    fd_set specialfds;
    std::multimap<int, CallbackInfo> read_listeners;
    std::multimap<int, CallbackInfo> write_listeners;
    std::multimap<int, CallbackInfo> special_listeners;
    int maxfds;
};
} // namespace ioruntime

#endif // WEBSERV_IOEVENTHANDLER_HPP
