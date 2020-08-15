//
// Created by boat on 7/3/20.
//

#include "IoEventHandler.hpp"
#include "../util/mem_copy.hpp"
#include "GlobalIoEventHandler.hpp"

namespace ioruntime {
IoEventHandler::IoEventHandler()
{
    auto rguard = read_fds.lock();
    auto wguard = write_fds.lock();
    auto sguard = special_fds.lock();

    FD_ZERO(&*rguard);
    FD_ZERO(&*wguard);
    FD_ZERO(&*sguard);
}

void IoEventHandler::reactor_step()
{
    fd_set read_selected, write_selected, special_selected;

    {
        auto fds = read_fds.lock();
        util::mem_copy(read_selected, *fds);
    }
    {
        auto fds = write_fds.lock();
        util::mem_copy(write_selected, *fds);
    }
    {
        auto fds = special_fds.lock();
        util::mem_copy(special_selected, *fds);
    }

    struct timeval tv {
        0
    };

    int max;
    {
        auto fds = maxfds.lock();
        max = *fds;
    }

    int selected = select(max + 1, &read_selected, &write_selected, &special_selected, &tv);

    if (selected > 0) {
        for (int fd = 0; fd <= max; fd += 1) {
            fire_listeners_for(fd, read_selected, read_listeners);
            fire_listeners_for(fd, write_selected, write_listeners);
            fire_listeners_for(fd, special_selected, special_listeners);
        }
    }
}

void IoEventHandler::register_reader_callback(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), read_listeners, read_fds, false);
}

void IoEventHandler::register_writer_callback(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), write_listeners, write_fds, false);
}

void IoEventHandler::register_special_callback(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), special_listeners, special_fds, false);
}

void IoEventHandler::unregister_reader_callbacks(int fd)
{
    auto listeners = read_listeners.lock();
    listeners->erase(fd);
}

void IoEventHandler::unregister_writer_callbacks(int fd)
{
    auto listeners = write_listeners.lock();
    listeners->erase(fd);
}

void IoEventHandler::unregister_special_callbacks(int fd)
{
    auto listeners = special_listeners.lock();
    listeners->erase(fd);
}

void IoEventHandler::register_reader_callback_once(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), read_listeners, read_fds, true);
}

void IoEventHandler::register_writer_callback_once(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), write_listeners, write_fds, true);
}

void IoEventHandler::register_special_callback_once(int fd, BoxFunctor&& x)
{
    register_callback(fd, std::move(x), special_listeners, special_fds, true);
}

void IoEventHandler::fire_listeners_for(int fd, fd_set& selected,
    Mutex<std::multimap<int, CallbackInfo>>& listeners_mutex)
{
    if (FD_ISSET(fd, &selected)) {
        auto listeners = listeners_mutex.lock();
        auto range = listeners->equal_range(fd);
        auto it = range.first;
        while (it != range.second) {
            (*it->second.bf)();
            if (it->second.once)
                it = listeners->erase(it);
            else
                ++it;
        }
    }
}

void IoEventHandler::register_callback(int fd,
    BoxFunctor&& x,
    Mutex<std::multimap<int, CallbackInfo>>& listeners,
    Mutex<fd_set>& set,
    bool once)
{
    {
        auto max = maxfds.lock();
        if (fd > *max)
            *max = fd;
    }
    {
        auto fds = set.lock();
        FD_SET(fd, &*fds);
    }
    auto guard = listeners.lock();
    guard->insert(std::pair<int, CallbackInfo>(fd, CallbackInfo { .once = once, .bf = std::move(x) }));
}

} // namespace ioruntime