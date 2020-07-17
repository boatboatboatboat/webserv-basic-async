cmake_minimum_required(VERSION 3.16)
project(webserv)

set(CMAKE_CXX_STANDARD 20)

add_executable(webserv main.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp util/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/EventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp)