//
// Created by boat on 7/3/20.
//

#include "IoEventHandler.hpp"
#include "../util/mem_copy.hpp"
#include "GlobalIoEventHandler.hpp"


namespace ioruntime {
IoEventHandler::IoEventHandler()
{
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&specialfds);
    maxfds = 0;
}

void IoEventHandler::reactor_step()
{
    fd_set read_selected, write_selected, special_selected;

    util::mem_copy(read_selected, readfds);
    util::mem_copy(write_selected, writefds);
    util::mem_copy(special_selected, specialfds);

    struct timeval tv {
        0
    };

    int selected = select(maxfds + 1, &read_selected, &write_selected, &special_selected, &tv);

    if (selected > 0) {
        for (int fd = 0; fd <= maxfds; fd += 1) {
            fire_listeners_for(fd, read_selected, read_listeners);
            fire_listeners_for(fd, write_selected, write_listeners);
            fire_listeners_for(fd, special_selected, special_listeners);
        }
    }
}

void IoEventHandler::register_reader_callback(int fd, BoxFunctor&& x)
{
    if (fd > maxfds)
        maxfds = fd;
    FD_SET(fd, &readfds);
    read_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
        .once = false,
        .bf = std::move(x)
    }));
}

void IoEventHandler::fire_listeners_for(int fd, fd_set& selected,
                        std::multimap<int, CallbackInfo>& listeners)
{
    if (FD_ISSET(fd, &selected)) {
        auto range = listeners.equal_range(fd);
        for (auto it = range.first; it != range.second; ++it) {
            // it = pair<int, Waker>, it->second() = Waker()
            (*it->second.bf)();
            if (it->second.once)
                listeners.erase(it);
        }
    }
}


void IoEventHandler::register_writer_callback(int fd, BoxFunctor&& x)
{
    if (fd > maxfds)
        maxfds = fd;
    FD_SET(fd, &writefds);
    write_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
            .once = false,
            .bf = std::move(x)
    }));
}

void IoEventHandler::register_special_callback(int fd, BoxFunctor&& x)
{
    if (fd > maxfds)
        maxfds = fd;
    FD_SET(fd, &specialfds);
    special_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
            .once = false,
            .bf = std::move(x)
    }));
}

void IoEventHandler::unregister_reader_callbacks(int fd)
{
    read_listeners.erase(fd);
}

void IoEventHandler::unregister_writer_callbacks(int fd)
{
    write_listeners.erase(fd);
}

void IoEventHandler::unregister_special_callbacks(int fd)
{
    special_listeners.erase(fd);
}

void IoEventHandler::register_reader_callback_once(int fd, BoxFunctor &&x) {
    if (fd > maxfds)
        maxfds = fd;
    FD_SET(fd, &readfds);
    read_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
            .once = true,
            .bf = std::move(x)
    }));
}

    void IoEventHandler::register_writer_callback_once(int fd, BoxFunctor &&x) {
        if (fd > maxfds)
            maxfds = fd;
        FD_SET(fd, &writefds);
        write_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
                .once = true,
                .bf = std::move(x)
        }));
    }

    void IoEventHandler::register_special_callback_once(int fd, BoxFunctor &&x) {
        if (fd > maxfds)
            maxfds = fd;
        FD_SET(fd, &specialfds);
        special_listeners.insert(std::pair<int, CallbackInfo>(fd, CallbackInfo {
                .once = true,
                .bf = std::move(x)
        }));
    }



} // namespace ioruntime