//
// Created by boat on 7/3/20.
//

#include "IoEventHandler.hpp"
#include "../utils/utils.hpp"
#include "GlobalIoEventHandler.hpp"
#include "GlobalTimeoutEventHandler.hpp"
#include <sys/select.h>

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
        utils::memcpy(read_selected, *fds);
    }
    {
        auto fds = write_fds.lock();
        utils::memcpy(write_selected, *fds);
    }
    {
        auto fds = special_fds.lock();
        utils::memcpy(special_selected, *fds);
    }

    struct timeval tv {
        .tv_sec = 0,
        .tv_usec = GlobalTimeoutEventHandler::is_clocks_empty() ? 5000 : 0
    };

    int max;
    {
        auto fds = maxfds.lock();
        max = *fds;
    }

    int selected = select(max + 1, &read_selected, &write_selected, &special_selected, &tv);
    if (selected > 0) {
        safe_fire_listeners(read_fd_in_use, read_selected, read_listeners);
        safe_fire_listeners(write_fd_in_use, write_selected, write_listeners);
        safe_fire_listeners(special_fd_in_use, special_selected, special_listeners);
    } else if (selected < 0) {
        DBGPRINT("Select failed: " << strerror(errno));
    }
}

void IoEventHandler::register_reader_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    {
        auto fds = read_fd_in_use.lock();
        auto pair = fds->find(fd);
        if (pair == fds->end()) {
            fds->insert(std::pair(fd, true));
        }
        TRACEPRINT("registered " << fd);
    }
    register_callback(fd, std::move(x), read_listeners, read_fds, once, unique);
}

void IoEventHandler::register_writer_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    {
        auto fds = write_fd_in_use.lock();
        auto pair = fds->find(fd);
        if (pair == fds->end()) {
            fds->insert(std::pair(fd, true));
        }
        TRACEPRINT("registered " << fd);
    }
    register_callback(fd, std::move(x), write_listeners, write_fds, once, unique);
}

void IoEventHandler::register_special_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    {
        auto fds = special_fd_in_use.lock();
        auto pair = fds->find(fd);
        if (pair == fds->end()) {
            fds->insert(std::pair(fd, true));
        }
    }
    register_callback(fd, std::move(x), special_listeners, special_fds, once, unique);
}

void IoEventHandler::unregister_reader_callbacks(int fd)
{
    {
        auto fds = read_fds.lock();
        FD_CLR(fd, &*fds);
    }
    {
        auto listeners = read_listeners.lock();
        listeners->erase(fd);
    }
    auto fds_in_use_guard = read_fd_in_use.lock();
    fds_in_use_guard->erase(fd);
}

void IoEventHandler::unregister_writer_callbacks(int fd)
{
    {
        auto fds = write_fds.lock();
        FD_CLR(fd, &*fds);
    }
    {
        auto listeners = write_listeners.lock();
        listeners->erase(fd);
    }
    auto fds_in_use_guard = write_fd_in_use.lock();
    fds_in_use_guard->erase(fd);
}

void IoEventHandler::unregister_special_callbacks(int fd)
{
    {
        auto fds = special_fds.lock();
        FD_CLR(fd, &*fds);
    }
    {
        auto listeners = special_listeners.lock();
        listeners->erase(fd);
    }
    auto fds_in_use_guard = special_fd_in_use.lock();
    fds_in_use_guard->erase(fd);
}

bool IoEventHandler::fire_listeners_for(int fd, fd_set& selected,
    Mutex<Listeners>& listeners_mutex)
{
    if (FD_ISSET(fd, &selected)) {
        // the IoEventHandler already has exclusive access over the
        // execution of the BoxFunctors, but not over the listeners.
        //
        // The CallbackInfo's should be moved out of the map so it
        // can let go of the listener_mutex lock, and then execute,
        // and then return the CallbackInfo's that weren't set to
        // run only once.
        std::multimap<int, CallbackInfo> cbinfos;
        {
            auto listeners = listeners_mutex.lock();
            auto range = listeners->equal_range(fd);
            auto it = range.first;
            while (it != range.second) {
                cbinfos.insert(std::move(*it));
                it->second.bf.leak(); // required because struct move
                it = listeners->erase(it);
            }
        }
        // Execute the moved-out CallbackInfo's,
        // and remove the once-flagged ones.
        {
            auto it = cbinfos.begin();
            while (it != cbinfos.end()) {
                TRACEPRINT("firing " << fd << "[" << it->second.once << ", " << it->second.unique << "]");
                (*it->second.bf)();
                if (it->second.once) {
                    it = cbinfos.erase(it);
                } else {
                    ++it;
                }
            }
        }
        // Move the CallbackInfo's back into the listeners.
        {
            auto listeners = listeners_mutex.lock();
            for (auto&& cbinfo : cbinfos) {
                listeners->insert(std::move(cbinfo));
            }
        }
        return true;
    } else {
        return false;
    }
}

void IoEventHandler::register_callback(int fd,
    BoxFunctor&& x,
    Mutex<Listeners>& listeners,
    Mutex<fd_set>& set,
    bool once,
    int unique)
{
    if (fd < 0) {
        throw std::runtime_error("IoEventHandler::register_callback: Negative file descriptor");
    }
    if (fd >= FD_SETSIZE) {
        throw std::runtime_error("IoEventHandler::register_callback: File descriptor >= FD_SETSIZE");
    }
    {
        auto max = maxfds.lock();
        if (fd > *max)
            *max = fd;
    }
    {
        auto fds = set.lock();
        FD_SET(fd, &*fds);
    }
    auto listeners_locked = listeners.lock();
    if (unique != 0) {
        // a unique key was passed, find & replace
        auto range = listeners_locked->equal_range(fd);
        auto it = range.first;
        while (it != range.second) {
            if (it->second.unique == unique) {
                it->second.bf = std::move(x);
                // there's only one unique - no need to continue
                return;
            }
            ++it;
        }
    }
    listeners_locked->insert(std::pair<int, CallbackInfo>(fd,
        CallbackInfo { .once = once, .bf = std::move(x), .unique = unique }));
}

void IoEventHandler::safe_fire_listeners(Mutex<std::map<int, bool>>& fds_in_use, fd_set& selected, Mutex<Listeners>& listeners)
{
    std::map<int, bool> fds;
    {
        auto fds_guard = fds_in_use.lock();
        fds = *fds_guard;
    }
    for (auto& fd_pair : fds) {
        auto fd = fd_pair.first;
        if (!fire_listeners_for(fd, selected, listeners)) {
#ifdef DEBUG_CHECK_IEH_FD_LEAKS
            DBGPRINT("possible file dscriptor leak: " << fd);
#endif
        }
    }
}

} // namespace ioruntime