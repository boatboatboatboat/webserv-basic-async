cmake_minimum_required(VERSION 3.16)
project(webserv)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g3 -Wall -Wextra -Werror -Wno-unknown-pragmas")

add_compile_definitions(
        LOG_DEBUG=1
        DEBUG
        SAFEPRINT_LINE_INFO
        _DARWIN_UNLIMITED_SELECT=1
        DEBUG_MUTEX_CANARY
        FD_SETSIZE=64
)


configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
# if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
# endif()

add_executable(webserv main.cpp http/HttpRequest.cpp http/HttpResponse.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp utils/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp net/TcpListener.cpp net/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp ioruntime/FdLineStream.cpp ioruntime/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp utils/dbg_puts.cpp utils/utils.hpp futures/IStreamExt.hpp futures/IStreamExt.ipp ioruntime/FileDescriptor.cpp ioruntime/FileDescriptor.hpp net/Socket.cpp net/Socket.hpp net/TcpStream.cpp net/TcpStream.hpp ioruntime/IAsyncRead.hpp ioruntime/IAsyncWrite.hpp net/SocketAddr.cpp net/SocketAddr.hpp http/HttpParser.cpp http/HttpParser.hpp http/ParserFuture.cpp http/ParserFuture.hpp http/HttpServer.ipp http/HttpServer.hpp http/StringBody.cpp http/StringBody.hpp ioruntime/IAsyncRead.cpp http/DefaultPageBody.cpp http/DefaultPageBody.hpp http/InfiniteBody.cpp http/InfiniteBody.hpp ioruntime/GlobalTimeoutEventHandler.cpp ioruntime/GlobalTimeoutEventHandler.hpp futures/SelectFuture.ipp futures/SelectFuture.hpp ioruntime/TimeoutFuture.cpp ioruntime/TimeoutFuture.hpp func/SetReadyFunctor.cpp func/SetReadyFunctor.hpp utils/mem_copy.cpp utils/mem_zero.cpp utils/mem_zero.hpp utils/cstr.hpp utils/cstr.cpp mutex/mutex.cpp http/HttpVersion.hpp http/HttpRfcConstants.hpp http/HttpStatus.hpp http/HttpStatus.cpp http/HttpVersion.cpp http/HttpHeader.cpp http/HttpHeader.hpp net/Ipv4Address.cpp net/Ipv4Address.hpp json/Json.cpp json/Json.hpp json/initializer_list.hpp fs/File.cpp fs/File.hpp ioruntime/FdStringReadFuture.cpp ioruntime/FdStringReadFuture.hpp config/Config.cpp config/Config.hpp net/Ipv6Address.cpp net/Ipv6Address.hpp net/IpAddress.cpp net/IpAddress.hpp http/HttpMethod.cpp http/HttpMethod.hpp http/DirectoryBody.cpp http/DirectoryBody.hpp regex/Regex.cpp regex/Regex.hpp)
add_executable(webserv_utests
        tests/rcptr_unittest.cpp
        tests/ipaddress_unittest.cpp
        http/HttpRequest.cpp http/HttpResponse.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp utils/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp net/TcpListener.cpp net/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp ioruntime/FdLineStream.cpp ioruntime/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp utils/dbg_puts.cpp utils/utils.hpp futures/IStreamExt.hpp futures/IStreamExt.ipp ioruntime/FileDescriptor.cpp ioruntime/FileDescriptor.hpp net/Socket.cpp net/Socket.hpp net/TcpStream.cpp net/TcpStream.hpp ioruntime/IAsyncRead.hpp ioruntime/IAsyncWrite.hpp net/SocketAddr.cpp net/SocketAddr.hpp http/HttpParser.cpp http/HttpParser.hpp http/ParserFuture.cpp http/ParserFuture.hpp http/HttpServer.ipp http/HttpServer.hpp http/StringBody.cpp http/StringBody.hpp ioruntime/IAsyncRead.cpp http/DefaultPageBody.cpp http/DefaultPageBody.hpp http/InfiniteBody.cpp http/InfiniteBody.hpp ioruntime/GlobalTimeoutEventHandler.cpp ioruntime/GlobalTimeoutEventHandler.hpp futures/SelectFuture.ipp futures/SelectFuture.hpp ioruntime/TimeoutFuture.cpp ioruntime/TimeoutFuture.hpp func/SetReadyFunctor.cpp func/SetReadyFunctor.hpp utils/mem_copy.cpp utils/mem_zero.cpp utils/mem_zero.hpp utils/cstr.hpp utils/cstr.cpp mutex/mutex.cpp http/HttpVersion.hpp http/HttpRfcConstants.hpp http/HttpStatus.hpp http/HttpStatus.cpp http/HttpVersion.cpp http/HttpHeader.cpp http/HttpHeader.hpp net/Ipv4Address.cpp net/Ipv4Address.hpp json/Json.cpp json/Json.hpp json/initializer_list.hpp fs/File.cpp fs/File.hpp ioruntime/FdStringReadFuture.cpp ioruntime/FdStringReadFuture.hpp config/Config.cpp config/Config.hpp net/Ipv6Address.cpp net/Ipv6Address.hpp net/IpAddress.cpp net/IpAddress.hpp tests/ipaddress_unittest.cpp)
target_link_libraries(webserv_utests gtest gtest_main)
enable_testing()
add_test( webserv_utests webserv_utests )