cmake_minimum_required(VERSION 3.17)
project(asynctest2)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(asynctest2 main.cpp rc_ptr.cpp rc_ptr.hpp box_ptr.cpp box_ptr.hpp mutex.cpp mutex.hpp)
target_link_libraries(asynctest2 PRIVATE Threads::Threads)
target_compile_options(asynctest2 PRIVATE -Wall -Wextra -Werror -fsanitize=address)
target_link_options(asynctest2 PRIVATE -fsanitize=address)