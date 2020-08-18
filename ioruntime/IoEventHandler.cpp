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
        .tv_sec = 1,
        .tv_usec = 0
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

void IoEventHandler::register_reader_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    register_callback(fd, std::move(x), read_listeners, read_fds, once, unique);
}

void IoEventHandler::register_writer_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    register_callback(fd, std::move(x), write_listeners, write_fds, once, unique);
}

void IoEventHandler::register_special_callback(int fd, BoxFunctor&& x, bool once, int unique)
{
    register_callback(fd, std::move(x), special_listeners, special_fds, once, unique);
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

void IoEventHandler::fire_listeners_for(int fd, fd_set& selected,
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
                it->second.bf.leak(); // TODO: make this not a hack
                it = listeners->erase(it);
            }
        }
        // Execute the moved-out CallbackInfo's,
        // and remove the once-flagged ones.
        {
            auto it = cbinfos.begin();
            while (it != cbinfos.end()) {
                std::stringstream x;
                x << "firing " << fd << "[" << it->second.once << ", " << it->second.unique << "]";
                //            	DBGPRINT(x.str());
                //
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
    }
}

void IoEventHandler::register_callback(int fd,
    BoxFunctor&& x,
    Mutex<Listeners>& listeners,
    Mutex<fd_set>& set,
    bool once,
    int unique)
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

} // namespace ioruntime