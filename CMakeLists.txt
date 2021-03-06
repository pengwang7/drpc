cmake_minimum_required(VERSION 3.9.6)

project(drpc C CXX)

set(CMAKE_BUILD_TYPE "Release")

#-Woverloaded-virtual
set(CXX_FLAGS
    -g
    -pipe
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -Wall
    -Wextra
    -Werror
    -Wclass-memaccess
    #-Wconversion
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
    -levent
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
#=======================================================#
    ${PROJECT_SOURCE_DIR}/src/json2pb/encode_decode.cc
    ${PROJECT_SOURCE_DIR}/src/json2pb/protobuf_map.cc
    ${PROJECT_SOURCE_DIR}/src/json2pb/pb_to_json.cc
    ${PROJECT_SOURCE_DIR}/src/json2pb/json_to_pb.cc
#=======================================================#
    ${PROJECT_SOURCE_DIR}/src/duration.cc
    ${PROJECT_SOURCE_DIR}/src/async_watcher.cc
    ${PROJECT_SOURCE_DIR}/src/socket.cc
    ${PROJECT_SOURCE_DIR}/src/async_socket.cc
    ${PROJECT_SOURCE_DIR}/src/listener.cc
    #${PROJECT_SOURCE_DIR}/src/timer_controller.cc
    #${PROJECT_SOURCE_DIR}/src/thread_pool.cc
    ${PROJECT_SOURCE_DIR}/src/event_loop.cc
    ${PROJECT_SOURCE_DIR}/src/event_loop_group.cc
    #${PROJECT_SOURCE_DIR}/src/scheduled.cc
    ${PROJECT_SOURCE_DIR}/src/channel.cc
    ${PROJECT_SOURCE_DIR}/src/rpc_channel.cc
    ${PROJECT_SOURCE_DIR}/src/server.cc
    #${PROJECT_SOURCE_DIR}/src/https/client.cc
    #${PROJECT_SOURCE_DIR}/test/test_logger.cc
    ${PROJECT_SOURCE_DIR}/test/test.cc
)

add_executable(drpc ${BUTIL_SRC} ${PROTO_SRC})

set(
    DYNAMIC_LIB
    ${LIBEVENT_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    spdlog::spdlog_header_only
    -fsanitize=address
)

target_link_libraries(drpc ${DYNAMIC_LIB})
