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
#include "../mutex/mutex.hpp"
#include "IEventHandler.hpp"

using futures::Waker;
using ioruntime::IEventHandler;
using mutex::Mutex;

namespace ioruntime {
class IoEventHandler : public IEventHandler {
public:
    IoEventHandler();
    void register_reader_callback(int fd, BoxFunctor&& x, bool once, int unique);
    void register_writer_callback(int fd, BoxFunctor&& x, bool once, int unique);
    void register_special_callback(int fd, BoxFunctor&& x, bool once, int unique);
    void unregister_reader_callbacks(int fd);
    void unregister_writer_callbacks(int fd);
    void unregister_special_callbacks(int fd);
    void reactor_step() override;

private:
    struct CallbackInfo {
        bool once;
        BoxFunctor bf;
        int unique;
    };
    typedef std::multimap<int, CallbackInfo> Listeners;
    static void fire_listeners_for(int fd, fd_set& selected, Mutex<Listeners>& listeners);
    void register_callback(int fd, BoxFunctor&& x, Mutex<Listeners>& listeners, Mutex<fd_set>& set, bool once, int unique);

    Mutex<fd_set> read_fds = Mutex(fd_set {});
    Mutex<fd_set> write_fds = Mutex(fd_set {});
    Mutex<fd_set> special_fds = Mutex(fd_set {});
    Mutex<Listeners> read_listeners = Mutex(Listeners());
    Mutex<Listeners> write_listeners = Mutex(Listeners());
    Mutex<Listeners> special_listeners = Mutex(Listeners());
    Mutex<int> maxfds = Mutex(0);
};
} // namespace ioruntime

#endif // WEBSERV_IOEVENTHANDLER_HPP
