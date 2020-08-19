cmake_minimum_required(VERSION 3.16)
project(webserv)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g3 -Wall -Wextra -Werror -Wno-unknown-pragmas")

add_compile_definitions(DEBUG)

add_executable(webserv main.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp util/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp ioruntime/TcpListener.cpp ioruntime/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp futures/FdLineStream.cpp futures/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp util/dbg_puts.cpp util/util.hpp)