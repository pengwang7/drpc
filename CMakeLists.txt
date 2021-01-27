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

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Libev)
if (LIBEV_FOUND)
    message(STATUS "found libev")
else(LIBEV_FOUND)
    message(FATAL_ERROR "not found libev")
endif()

find_package(Protobuf)
if(PROTOBUF_FOUND)
    include(FindProtobuf)
    message(STATUS "found protobuf")
else(PROTOBUF_FOUND)
    message(FATAL_ERROR "not found protobuf")
endif()

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
    message(STATUS "found spdlog")
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER proto/rpc.proto proto/service.proto)

set(
    BUTIL_SRC
    ${PROJECT_SOURCE_DIR}/src/socket.cpp
    ${PROJECT_SOURCE_DIR}/src/ip_address.cpp
    ${PROJECT_SOURCE_DIR}/src/listener.cpp
    ${PROJECT_SOURCE_DIR}/src/async_socket.cpp
    ${PROJECT_SOURCE_DIR}/src/async_watcher.cpp
    ${PROJECT_SOURCE_DIR}/src/event_loop.cpp
    ${PROJECT_SOURCE_DIR}/src/event_loop_group.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduled.cpp
    ${PROJECT_SOURCE_DIR}/src/channel.cpp
    ${PROJECT_SOURCE_DIR}/src/rpc_channel.cpp
    ${PROJECT_SOURCE_DIR}/src/server.cpp
    ${PROJECT_SOURCE_DIR}/test/test.cpp
    #${PROJECT_SOURCE_DIR}/test/client.cpp
)

add_executable(drpc ${BUTIL_SRC} ${PROTO_SRC})

set(
    DYNAMIC_LIB
    ${LIBEV_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    spdlog::spdlog_header_only
    -fsanitize=address
)

target_link_libraries(drpc ${DYNAMIC_LIB})
