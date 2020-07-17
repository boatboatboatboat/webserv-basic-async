//
// Created by boat on 12-07-20.
//

#ifndef WEBSERV_THREADLESSEXECUTOR_HPP
#define WEBSERV_THREADLESSEXECUTOR_HPP

#include "ioruntime.hpp"

namespace ioruntime {
    class ThreadlessExecutor: public IExecutor {
    public:
        ThreadlessExecutor();
        void spawn(BoxPtr<IFuture<void>>&& future) override;
        void step() override;
    private:
        std::vector<BoxPtr<IFuture<void>>> tasks;
    };
}

#endif //WEBSERV_THREADLESSEXECUTOR_HPP
