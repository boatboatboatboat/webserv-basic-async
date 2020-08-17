#include <iostream>
#include <fcntl.h>
#include "ioruntime/ioruntime.hpp"

#include "futures/FdLineStream.hpp"
#include "futures/ForEachFuture.hpp"
#include "util/util.hpp"

using ioruntime::GlobalRuntime;
using ioruntime::GlobalIoEventHandler;
using ioruntime::Runtime;
using ioruntime::RuntimeBuilder;
using futures::IFuture;
using futures::PollResult;
using futures::ForEachFuture;
using futures::FdLineStream;

/*
class FileDescriptor {
public:
	[[nodiscard]] bool has_data_to_read() const {
		return can_read;
	}
	[[nodiscard]] bool has_data_to_write() const {
		return can_write;
	}
private:
	bool can_read;
	bool can_write;
};
 */
/*
class GetLine: public IFuture<void> {
    class SetReadyFunctor: public Functor {
    public:
        SetReadyFunctor(bool* cr_source): cread(cr_source) {}
        void operator()() override {
            *cread = true;
        }
    private:
        bool* cread;
    };
public:
    GetLine() {
        // code looks stupid because of C++ template limitation:
        // there's only template typechecking for *this exact case*
        // fixme: could be fixed with having an assignment operator with two type parameters
        BoxPtr<Functor> ptr = BoxPtr<Functor>(new SetReadyFunctor(&can_read));
        can_read = false;
        GlobalIoEventHandler::register_reader_callback(
                STDIN_FILENO, std::move(ptr));
        // TODO: error handling for fcntl
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    }
    ~GetLine() {
        // clean up our read-ready callback
        GlobalIoEventHandler::unregister_reader_callbacks(STDIN_FILENO);
    }
	PollResult<void> poll(Waker&& waker) override {
		if (can_read) {
		    // invalidate our read state
		    can_read = false;
		    //read
		    int res = read(STDIN_FILENO, reading_buffer, 24);
            switch (res) {
                case -1: {
                    // Error during read
                    std::cout << "eRROR" << std::endl;
                    return PollResult<void>::ready();
                } break;
                case 0: {
                    std::cout << buffer << std::endl;
                    return PollResult<void>::ready();
                } break;
                default: {
                    for (int i = 0; i < res; i += 1) {
                        // if newline, return result
                        if (reading_buffer[i] == '\n') {
                            std::cout << "res: " << buffer << std::endl;
                            return PollResult<void>::ready();
                        }
                        // otherwise add to buffer
                        buffer += reading_buffer[i];
                    }
                    // Register 'once' -> after execution, discard the waker.
                    std::cout << "Reregistered in p" << std::endl;
                    GlobalIoEventHandler::register_reader_callback_once(STDIN_FILENO, waker.boxed());
                    return PollResult<void>::pending();
                } break;
            }
		} else {
		    // Register 'once' -> after execution, discard the waker.
            GlobalIoEventHandler::register_reader_callback_once(STDIN_FILENO, waker.boxed());
            return PollResult<void>::pending();
		}
	}
private:
	bool can_read = false;
    std::string buffer;
    char reading_buffer[24];
};
*/
using ioruntime::IoEventHandler;

int core() {
	auto runtime = RuntimeBuilder()
		.with_workers(6)
		//.without_workers()
		.build();

	auto file = open("test.txt", O_RDONLY);
	auto file2 = open("test2.txt", O_RDONLY);
	auto io_event = BoxPtr<IoEventHandler>::make();
	runtime.register_io_handler(std::move(io_event));
	GlobalRuntime::set_runtime(&runtime);
	GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
	        std::move(FdLineStream(STDIN_FILENO)), [](std::string& x)
	        {
	            std::string y("in line: ");
	            y.append(x);
	            DBGPRINT(y);
	        }));
    GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
            std::move(FdLineStream(file)), [](std::string& x)
            {
                std::string y("file line: ");
                y.append(x);
                DBGPRINT(y);
            }));
    GlobalRuntime::spawn(BoxPtr<ForEachFuture<FdLineStream, std::string>>::make(
            std::move(FdLineStream(file2)), [](std::string& x)
            {
                std::string y("file2 line: ");
                y.append(x);
                DBGPRINT(y);
            }));
	runtime.naive_run();
	return 0;
}

int main() {
    return (core());
}