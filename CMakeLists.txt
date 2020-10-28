cmake_minimum_required(VERSION 3.16)
project(webserv)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g3 -Wall -Wextra -Werror -Wno-unknown-pragmas")

add_compile_definitions(
        LOG_DEBUG=1
        DEBUG
        SAFEPRINT_LINE_INFO
        _DARWIN_UNLIMITED_SELECT=1
        DEBUG_MUTEX_CANARY
        EVIL_CHECK_ERRNO
        #DEBUG_REQUEST_BODY_ALWAYS_SWITCH
        # DEBUG_SOCKET_INCOMING_PRINTER
        #DEBUG_SOCKET_OUTGOING_PRINTER
        #DEBUG_FILEDESCRIPTOR_OUTGOING_PRINTER
        #FD_SETSIZE=64
)


configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

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

include_directories("langmod/lua")

add_executable(webserv
        # lua
        langmod/lua/lapi.c
        langmod/lua/lauxlib.c
        langmod/lua/lbaselib.c
        langmod/lua/lcode.c
        langmod/lua/lcorolib.c
        langmod/lua/lctype.c
        langmod/lua/ldblib.c
        langmod/lua/ldebug.c
        langmod/lua/ldo.c
        langmod/lua/ldump.c
        langmod/lua/lfunc.c
        langmod/lua/lgc.c
        langmod/lua/linit.c
        langmod/lua/liolib.c
        langmod/lua/llex.c
        langmod/lua/lmathlib.c
        langmod/lua/lmem.c
        langmod/lua/loadlib.c
        langmod/lua/lobject.c
        langmod/lua/lopcodes.c
        langmod/lua/loslib.c
        langmod/lua/lparser.c
        langmod/lua/lstate.c
        langmod/lua/lstring.c
        langmod/lua/lstrlib.c
        langmod/lua/ltable.c
        langmod/lua/ltablib.c
        langmod/lua/ltm.c
        langmod/lua/lundump.c
        langmod/lua/lutf8lib.c
        langmod/lua/lvm.c
        langmod/lua/lzio.c
        # javascript
        langmod/es/duktape.c
        langmod/es/duk_print_alert.c
        # webserv
        main.cpp http/OutgoingResponse.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp utils/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp net/TcpListener.cpp net/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp ioruntime/FdLineStream.cpp ioruntime/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp utils/dbg_puts.cpp utils/utils.hpp futures/IStreamExt.hpp futures/IStreamExt.ipp ioruntime/FileDescriptor.cpp ioruntime/FileDescriptor.hpp net/Socket.cpp net/Socket.hpp net/TcpStream.cpp net/TcpStream.hpp ioruntime/IAsyncRead.hpp ioruntime/IAsyncWrite.hpp net/SocketAddr.cpp net/SocketAddr.hpp http/Server.hpp http/StringReader.cpp http/StringReader.hpp ioruntime/IAsyncRead.cpp http/DefaultPageReader.cpp http/DefaultPageReader.hpp http/InfiniteReader.cpp http/InfiniteReader.hpp ioruntime/GlobalTimeoutEventHandler.cpp ioruntime/GlobalTimeoutEventHandler.hpp futures/SelectFuture.ipp futures/SelectFuture.hpp ioruntime/TimeoutFuture.cpp ioruntime/TimeoutFuture.hpp func/SetReadyFunctor.cpp func/SetReadyFunctor.hpp utils/mem_copy.cpp utils/mem_zero.cpp utils/mem_zero.hpp utils/cstr.hpp utils/cstr.cpp mutex/mutex.cpp http/Version.hpp http/RfcConstants.hpp http/Status.hpp http/Status.cpp http/Version.cpp http/Header.cpp http/Header.hpp net/Ipv4Address.cpp net/Ipv4Address.hpp json/Json.cpp json/Json.hpp json/initializer_list.hpp fs/File.cpp fs/File.hpp ioruntime/FdStringReadFuture.cpp ioruntime/FdStringReadFuture.hpp config/Config.cpp config/Config.hpp net/Ipv6Address.cpp net/Ipv6Address.hpp net/IpAddress.cpp net/IpAddress.hpp http/Method.cpp http/Method.hpp http/DirectoryReader.cpp http/DirectoryReader.hpp regex/Regex.cpp regex/Regex.hpp http/Uri.cpp http/Uri.hpp args/Args.cpp args/Args.hpp cgi/Cgi.cpp cgi/Cgi.hpp ioruntime/GlobalChildProcessHandler.cpp ioruntime/GlobalChildProcessHandler.hpp func/SetValueFunctor.ipp func/SetValueFunctor.hpp ioruntime/IoCopyFuture.ipp ioruntime/IoCopyFuture.hpp http/SpanReader.cpp http/SpanReader.hpp ioruntime/IAsyncWrite.cpp ioruntime/IoResult.cpp ioruntime/IoResult.hpp http/Server.cpp http/OutgoingBody.cpp http/OutgoingBody.hpp constants/config.hpp fs/Path.cpp fs/Path.hpp http/FutureSeReader.cpp http/FutureSeReader.hpp http/IncomingBody.cpp http/IncomingBody.hpp constants/mimetypes.hpp utils/localtime.hpp utils/localtime.cpp utils/base64.cpp utils/base64.hpp utils/monostate.hpp langmod/ScriptReader.cpp langmod/ScriptReader.hpp langmod/LuaLanguage.cpp langmod/LuaLanguage.hpp langmod/ESLanguage.cpp langmod/ESLanguage.hpp http/MessageParser.cpp http/MessageParser.hpp ioruntime/CharacterStream.cpp ioruntime/CharacterStream.hpp http/IncomingMessage.cpp http/IncomingMessage.hpp http/IncomingRequest.cpp http/IncomingRequest.hpp http/RequestParser.cpp http/RequestParser.hpp http/IncomingResponse.cpp http/IncomingResponse.hpp http/ResponseParser.cpp http/ResponseParser.hpp utils/StringStream.cpp utils/StringStream.hpp http/Proxy.hpp http/Proxy.cpp http/OutgoingRequest.cpp http/OutgoingRequest.hpp http/OutgoingMessage.cpp http/OutgoingMessage.hpp http/MessageReader.cpp http/MessageReader.hpp http/RequestReader.cpp http/RequestReader.hpp http/ServerHandlerResponse.cpp http/ServerHandlerResponse.hpp utils/MemCompare.cpp utils/MemCompare.hpp)
add_executable(webserv_utests
        langmod/lua/lapi.c
        langmod/lua/lauxlib.c
        langmod/lua/lbaselib.c
        langmod/lua/lcode.c
        langmod/lua/lcorolib.c
        langmod/lua/lctype.c
        langmod/lua/ldblib.c
        langmod/lua/ldebug.c
        langmod/lua/ldo.c
        langmod/lua/ldump.c
        langmod/lua/lfunc.c
        langmod/lua/lgc.c
        langmod/lua/linit.c
        langmod/lua/liolib.c
        langmod/lua/llex.c
        langmod/lua/lmathlib.c
        langmod/lua/lmem.c
        langmod/lua/loadlib.c
        langmod/lua/lobject.c
        langmod/lua/lopcodes.c
        langmod/lua/loslib.c
        langmod/lua/lparser.c
        langmod/lua/lstate.c
        langmod/lua/lstring.c
        langmod/lua/lstrlib.c
        langmod/lua/ltable.c
        langmod/lua/ltablib.c
        langmod/lua/ltm.c
        langmod/lua/lundump.c
        langmod/lua/lutf8lib.c
        langmod/lua/lvm.c
        langmod/lua/lzio.c
        # javascript
        langmod/es/duktape.c
        langmod/es/duk_print_alert.c
        tests/rcptr_unittest.cpp
        tests/ipaddress_unittest.cpp
        tests/optional_unittest.cpp
        tests/uri_unittest.cpp
        tests/ResponseReader_unittest.cpp
        tests/RequestParser_unittest.cpp
        tests/Base64_unittest.cpp
        tests/span_unittest.cpp
        http/OutgoingResponse.cpp futures/PollResult.hpp futures/futures.hpp futures/IFuture.hpp futures/Waker.hpp futures/Waker.cpp boxed/BoxPtr.hpp futures/PollResult.cpp utils/mem_copy.hpp ioruntime/IoEventHandler.cpp ioruntime/IoEventHandler.hpp func/Functor.hpp ioruntime/IEventHandler.hpp ioruntime/Runtime.cpp ioruntime/Runtime.hpp ioruntime/IExecutor.hpp ioruntime/PooledExecutor.cpp ioruntime/PooledExecutor.hpp ioruntime/ThreadlessExecutor.cpp ioruntime/ThreadlessExecutor.hpp ioruntime/ioruntime.hpp ioruntime/RuntimeBuilder.cpp ioruntime/RuntimeBuilder.hpp ioruntime/GlobalRuntime.cpp ioruntime/GlobalRuntime.hpp boxed/RcPtr.hpp mutex/mutex.hpp mutex/mutex.ipp ioruntime/IEventHandler.cpp ioruntime/IExecutor.cpp func/Functor.cpp ioruntime/GlobalIoEventHandler.cpp ioruntime/GlobalIoEventHandler.hpp ioruntime/TimeoutEventHandler.cpp ioruntime/TimeoutEventHandler.hpp net/TcpListener.cpp net/TcpListener.hpp futures/IStream.hpp futures/StreamPollResult.hpp futures/StreamPollResult.ipp futures/ForEachFuture.cpp futures/ForEachFuture.hpp ioruntime/FdLineStream.cpp ioruntime/FdLineStream.hpp futures/Task.cpp futures/Task.hpp boxed/boxed.hpp ioruntime/GlobalRuntime.ipp utils/dbg_puts.cpp utils/utils.hpp futures/IStreamExt.hpp futures/IStreamExt.ipp ioruntime/FileDescriptor.cpp ioruntime/FileDescriptor.hpp net/Socket.cpp net/Socket.hpp net/TcpStream.cpp net/TcpStream.hpp ioruntime/IAsyncRead.hpp ioruntime/IAsyncWrite.hpp net/SocketAddr.cpp net/SocketAddr.hpp http/Server.hpp http/StringReader.cpp http/StringReader.hpp ioruntime/IAsyncRead.cpp http/DefaultPageReader.cpp http/DefaultPageReader.hpp http/InfiniteReader.cpp http/InfiniteReader.hpp ioruntime/GlobalTimeoutEventHandler.cpp ioruntime/GlobalTimeoutEventHandler.hpp futures/SelectFuture.ipp futures/SelectFuture.hpp ioruntime/TimeoutFuture.cpp ioruntime/TimeoutFuture.hpp func/SetReadyFunctor.cpp func/SetReadyFunctor.hpp utils/mem_copy.cpp utils/mem_zero.cpp utils/mem_zero.hpp utils/cstr.hpp utils/cstr.cpp mutex/mutex.cpp http/Version.hpp http/RfcConstants.hpp http/Status.hpp http/Status.cpp http/Version.cpp http/Header.cpp http/Header.hpp net/Ipv4Address.cpp net/Ipv4Address.hpp json/Json.cpp json/Json.hpp json/initializer_list.hpp fs/File.cpp fs/File.hpp ioruntime/FdStringReadFuture.cpp ioruntime/FdStringReadFuture.hpp config/Config.cpp config/Config.hpp net/Ipv6Address.cpp net/Ipv6Address.hpp net/IpAddress.cpp net/IpAddress.hpp http/Method.cpp http/Method.hpp http/DirectoryReader.cpp http/DirectoryReader.hpp regex/Regex.cpp regex/Regex.hpp http/Uri.cpp http/Uri.hpp args/Args.cpp args/Args.hpp cgi/Cgi.cpp cgi/Cgi.hpp ioruntime/GlobalChildProcessHandler.cpp ioruntime/GlobalChildProcessHandler.hpp func/SetValueFunctor.ipp func/SetValueFunctor.hpp ioruntime/IoCopyFuture.ipp ioruntime/IoCopyFuture.hpp http/SpanReader.cpp http/SpanReader.hpp ioruntime/IAsyncWrite.cpp ioruntime/IoResult.cpp ioruntime/IoResult.hpp http/Server.cpp http/OutgoingBody.cpp http/OutgoingBody.hpp constants/config.hpp fs/Path.cpp fs/Path.hpp http/FutureSeReader.cpp http/FutureSeReader.hpp http/IncomingBody.cpp http/IncomingBody.hpp constants/mimetypes.hpp utils/localtime.hpp utils/localtime.cpp utils/base64.cpp utils/base64.hpp utils/monostate.hpp langmod/ScriptReader.cpp langmod/ScriptReader.hpp langmod/LuaLanguage.cpp langmod/LuaLanguage.hpp langmod/ESLanguage.cpp langmod/ESLanguage.hpp http/MessageParser.cpp http/MessageParser.hpp ioruntime/CharacterStream.cpp ioruntime/CharacterStream.hpp http/IncomingMessage.cpp http/IncomingMessage.hpp http/IncomingRequest.cpp http/IncomingRequest.hpp http/RequestParser.cpp http/RequestParser.hpp)
target_link_libraries(webserv_utests gtest gtest_main)
enable_testing()
add_test(webserv_utests webserv_utests)