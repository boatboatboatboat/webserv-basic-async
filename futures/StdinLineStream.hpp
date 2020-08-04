//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STDINLINESTREAM_HPP
#define WEBSERV_FUTURES_STDINLINESTREAM_HPP

#include "../boxed/RcPtr.hpp"
#include "futures.hpp"
#include <string>

using boxed::RcPtr;

namespace futures {
class StdinLineStream : public IStream<std::string> {
    class SetReadyFunctor : public Functor {
    public:
        SetReadyFunctor(bool* cr_source)
            : cread(cr_source)
        {
        }
        void operator()() override { *cread = true; }

    private:
        bool* cread;
    };

public:
    StdinLineStream()
    {
        RcPtr<Functor> ptr = RcPtr<Functor>(std::move(SetReadyFunctor(&can_read)));
    }
    ~StdinLineStream()
    {
    }

private:
    bool can_read;
};
}

#endif //WEBSERV_FUTURES_STDINLINESTREAM_HPP
