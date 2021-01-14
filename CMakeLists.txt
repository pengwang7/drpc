cmake_minimum_required(VERSION 3.9.6)

project(drpc C CXX)

set(CMAKE_BUILD_TYPE "Release")

#-Woverloaded-virtual
set(CXX_FLAGS
    -g
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wno-unused-parameter
    -Wpointer-arith
    -Wwrite-strings
    -march=native
    -fno-omit-frame-pointer
    -std=c++11
    -rdynamic
    -lpthread
    -ldl
    -lm
    -lev
    -fsanitize=address
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -DNDEBUG") 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include_directories(${PROJECT_SOURCE_DIR}/src)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_executable(drpc
    src/socket.cpp
    src/ip_address.cpp
    src/listener.cpp
    src/async_socket.cpp
    src/async_watcher.cpp
    src/event_loop.cpp
    src/event_loop_group.cpp
    src/scheduled.cpp
    src/channel.cpp
    src/server.cpp
    test/test.cpp
)

target_link_libraries(drpc -fsanitize=address)

find_package(Libev)
if (LIBEV_FOUND)
    message(STATUS "found libev")
    target_include_directories(drpc PRIVATE ${LIBEV_INCLUDE})
    target_link_libraries(drpc ${LIBEV_LIBRARY})
else(LIBEV_FOUND)
    message(FATAL_ERROR "not found libev")
endif()

find_package(Protobuf)
if(PROTOBUF_FOUND)
    message(STATUS "found protobuf")
else(PROTOBUF_FOUND)
    message(FATAL_ERROR "not found protobuf") 
endif()

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
    message(STATUS "found spdlog")
    target_link_libraries(drpc spdlog::spdlog_header_only)
endif()
