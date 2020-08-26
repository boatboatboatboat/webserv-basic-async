cmake_minimum_required(VERSION 3.16)
project(webserv)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g3 -Wall -Wextra -Werror -Wno-unknown-pragmas")

add_compile_definitions(
        LOG_INFO=1
        DEBUG
        SAFEPRINT_LINE_INFO
        _DARWIN_UNLIMITED_SELECT=1
        DEBUG_MUTEX_CANARY
        FD_SETSIZE=64
)

add_executable(webserv main.cpp http/HttpRequest.cpp http/HttpResponse.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp utils/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp ioruntime/TcpListener.cpp ioruntime/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp ioruntime/FdLineStream.cpp ioruntime/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp utils/dbg_puts.cpp utils/utils.hpp futures/IStreamExt.hpp futures/IStreamExt.ipp ioruntime/FileDescriptor.cpp ioruntime/FileDescriptor.hpp ioruntime/Socket.cpp ioruntime/Socket.hpp ioruntime/TcpStream.cpp ioruntime/TcpStream.hpp ioruntime/IAsyncRead.hpp ioruntime/IAsyncWrite.hpp ioruntime/SocketAddr.cpp ioruntime/SocketAddr.hpp http/HttpParser.cpp http/HttpParser.hpp http/ParserFuture.cpp http/ParserFuture.hpp http/HttpServer.ipp http/HttpServer.hpp http/StringBody.cpp http/StringBody.hpp ioruntime/IAsyncRead.cpp http/DefaultPageBody.cpp http/DefaultPageBody.hpp http/InfiniteBody.cpp http/InfiniteBody.hpp ioruntime/GlobalTimeoutEventHandler.cpp ioruntime/GlobalTimeoutEventHandler.hpp futures/SelectFuture.ipp futures/SelectFuture.hpp ioruntime/TimeoutFuture.cpp ioruntime/TimeoutFuture.hpp func/SetReadyFunctor.cpp func/SetReadyFunctor.hpp)