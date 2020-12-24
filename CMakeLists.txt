cmake_minimum_required(VERSION 3.9.6)

project(drpc)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0 -fsanitize=address -fno-omit-frame-pointer -DCHECK_PTHREAD_RETURN_VALUE -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0 -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-aliasing -fstrict-aliasing -fno-omit-frame-pointer -g -march=native -pipe")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -Wextra -Werror -Wno-unused-parameter -Woverloaded-virtual -Wpointer-arith")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wwrite-strings -Wno-invalid-offsetof ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -lcurl")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -g")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address -g")

set(BUTIL_SOURCES
    ${PROJECT_SOURCE_DIR}/test/test.cpp
)

add_executable(${PROJECT_NAME} ${BUTIL_SOURCES})

target_link_libraries(drpc -fsanitize=address)
#target_link_libraries(dict /home/pengwang/cloud_platform/target/3rd/gflags/libgflags.so)
#target_link_libraries(curl /home/pengwang/cloud_platform/target/3rd/glog/libglog.a)
