//
// Created by Djevayo Pattij on 7/18/20.
//

#include "GlobalIoEventHandler.hpp"

namespace ioruntime {
    Mutex<IoEventHandler*> GlobalIoEventHandler::event_handler(
            reinterpret_cast<ioruntime::IoEventHandler*>(0x123456789));

    void GlobalIoEventHandler::register_reader_callback(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_reader_callback(fd, std::move(x));
    }

    void GlobalIoEventHandler::register_writer_callback(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_writer_callback(fd, std::move(x));

    }

    void GlobalIoEventHandler::register_special_callback(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_special_callback(fd, std::move(x));

    }

    void GlobalIoEventHandler::unregister_reader_callbacks(int fd) {
        auto guard = event_handler.lock();
        guard.get()->unregister_reader_callbacks(fd);
    }

    void GlobalIoEventHandler::unregister_writer_callbacks(int fd) {
        auto guard = event_handler.lock();
        guard.get()->unregister_writer_callbacks(fd);

    }

    void GlobalIoEventHandler::unregister_special_callbacks(int fd) {
        auto guard = event_handler.lock();
        guard.get()->unregister_special_callbacks(fd);
    }

    void GlobalIoEventHandler::set(IoEventHandler *handler) {
        auto ev = event_handler.lock();
        *ev = handler;
    }

    void GlobalIoEventHandler::register_reader_callback_once(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_reader_callback_once(fd, std::move(x));
    }

    void GlobalIoEventHandler::register_writer_callback_once(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_writer_callback_once(fd, std::move(x));
    }

    void GlobalIoEventHandler::register_special_callback_once(int fd, BoxFunctor &&x) {
        auto guard = event_handler.lock();
        guard.get()->register_special_callback_once(fd, std::move(x));
    }
}