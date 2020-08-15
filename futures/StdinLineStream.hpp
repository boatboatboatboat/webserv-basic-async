//
// Created by Djevayo Pattij on 8/3/20.
//

#ifndef WEBSERV_FUTURES_STDINLINESTREAM_HPP
#define WEBSERV_FUTURES_STDINLINESTREAM_HPP

#include "../boxed/RcPtr.hpp"
#include "futures.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include <string>
#include <fcntl.h>

using boxed::RcPtr;
using ioruntime::GlobalIoEventHandler;

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
    	buffer = "";
        //RcPtr<Functor> ptr = RcPtr<Functor>(std::move(SetReadyFunctor(&can_read)));
    	BoxFunctor ptr = BoxFunctor(new SetReadyFunctor(&can_read));
    	GlobalIoEventHandler::register_reader_callback(STDIN_FILENO, std::move(ptr));
    	// TODO: err handling for fcntl
    	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    }
    ~StdinLineStream()
    {
		GlobalIoEventHandler::unregister_reader_callbacks(STDIN_FILENO);
	}
	StreamPollResult<std::string> poll_next(Waker&& waker) {
    	if (can_read) {
			can_read = false;

			// we're treating stdin as a semi-regular file
			// stdin *never* reaches eof
			int res = read(STDIN_FILENO, reading_buffer, sizeof(reading_buffer));
			if (res < 0) {

			} else {

			}
    	} else {

    	}
    }

private:
    bool can_read = false;
    std::string buffer;
    char reading_buffer[64];
};
}

#endif //WEBSERV_FUTURES_STDINLINESTREAM_HPP
