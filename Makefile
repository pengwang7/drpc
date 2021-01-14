# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pengwang/workspace/github/drpc

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pengwang/workspace/github/drpc

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/local/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/pengwang/workspace/github/drpc/CMakeFiles /home/pengwang/workspace/github/drpc/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/pengwang/workspace/github/drpc/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named drpc

# Build rule for target.
drpc: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 drpc
.PHONY : drpc

# fast build rule for target.
drpc/fast:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/build
.PHONY : drpc/fast

src/async_socket.o: src/async_socket.cpp.o

.PHONY : src/async_socket.o

# target to build an object file
src/async_socket.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_socket.cpp.o
.PHONY : src/async_socket.cpp.o

src/async_socket.i: src/async_socket.cpp.i

.PHONY : src/async_socket.i

# target to preprocess a source file
src/async_socket.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_socket.cpp.i
.PHONY : src/async_socket.cpp.i

src/async_socket.s: src/async_socket.cpp.s

.PHONY : src/async_socket.s

# target to generate assembly for a file
src/async_socket.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_socket.cpp.s
.PHONY : src/async_socket.cpp.s

src/async_watcher.o: src/async_watcher.cpp.o

.PHONY : src/async_watcher.o

# target to build an object file
src/async_watcher.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_watcher.cpp.o
.PHONY : src/async_watcher.cpp.o

src/async_watcher.i: src/async_watcher.cpp.i

.PHONY : src/async_watcher.i

# target to preprocess a source file
src/async_watcher.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_watcher.cpp.i
.PHONY : src/async_watcher.cpp.i

src/async_watcher.s: src/async_watcher.cpp.s

.PHONY : src/async_watcher.s

# target to generate assembly for a file
src/async_watcher.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/async_watcher.cpp.s
.PHONY : src/async_watcher.cpp.s

src/channel.o: src/channel.cpp.o

.PHONY : src/channel.o

# target to build an object file
src/channel.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/channel.cpp.o
.PHONY : src/channel.cpp.o

src/channel.i: src/channel.cpp.i

.PHONY : src/channel.i

# target to preprocess a source file
src/channel.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/channel.cpp.i
.PHONY : src/channel.cpp.i

src/channel.s: src/channel.cpp.s

.PHONY : src/channel.s

# target to generate assembly for a file
src/channel.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/channel.cpp.s
.PHONY : src/channel.cpp.s

src/event_loop.o: src/event_loop.cpp.o

.PHONY : src/event_loop.o

# target to build an object file
src/event_loop.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop.cpp.o
.PHONY : src/event_loop.cpp.o

src/event_loop.i: src/event_loop.cpp.i

.PHONY : src/event_loop.i

# target to preprocess a source file
src/event_loop.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop.cpp.i
.PHONY : src/event_loop.cpp.i

src/event_loop.s: src/event_loop.cpp.s

.PHONY : src/event_loop.s

# target to generate assembly for a file
src/event_loop.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop.cpp.s
.PHONY : src/event_loop.cpp.s

src/event_loop_group.o: src/event_loop_group.cpp.o

.PHONY : src/event_loop_group.o

# target to build an object file
src/event_loop_group.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop_group.cpp.o
.PHONY : src/event_loop_group.cpp.o

src/event_loop_group.i: src/event_loop_group.cpp.i

.PHONY : src/event_loop_group.i

# target to preprocess a source file
src/event_loop_group.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop_group.cpp.i
.PHONY : src/event_loop_group.cpp.i

src/event_loop_group.s: src/event_loop_group.cpp.s

.PHONY : src/event_loop_group.s

# target to generate assembly for a file
src/event_loop_group.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/event_loop_group.cpp.s
.PHONY : src/event_loop_group.cpp.s

src/ip_address.o: src/ip_address.cpp.o

.PHONY : src/ip_address.o

# target to build an object file
src/ip_address.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/ip_address.cpp.o
.PHONY : src/ip_address.cpp.o

src/ip_address.i: src/ip_address.cpp.i

.PHONY : src/ip_address.i

# target to preprocess a source file
src/ip_address.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/ip_address.cpp.i
.PHONY : src/ip_address.cpp.i

src/ip_address.s: src/ip_address.cpp.s

.PHONY : src/ip_address.s

# target to generate assembly for a file
src/ip_address.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/ip_address.cpp.s
.PHONY : src/ip_address.cpp.s

src/listener.o: src/listener.cpp.o

.PHONY : src/listener.o

# target to build an object file
src/listener.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/listener.cpp.o
.PHONY : src/listener.cpp.o

src/listener.i: src/listener.cpp.i

.PHONY : src/listener.i

# target to preprocess a source file
src/listener.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/listener.cpp.i
.PHONY : src/listener.cpp.i

src/listener.s: src/listener.cpp.s

.PHONY : src/listener.s

# target to generate assembly for a file
src/listener.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/listener.cpp.s
.PHONY : src/listener.cpp.s

src/scheduled.o: src/scheduled.cpp.o

.PHONY : src/scheduled.o

# target to build an object file
src/scheduled.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/scheduled.cpp.o
.PHONY : src/scheduled.cpp.o

src/scheduled.i: src/scheduled.cpp.i

.PHONY : src/scheduled.i

# target to preprocess a source file
src/scheduled.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/scheduled.cpp.i
.PHONY : src/scheduled.cpp.i

src/scheduled.s: src/scheduled.cpp.s

.PHONY : src/scheduled.s

# target to generate assembly for a file
src/scheduled.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/scheduled.cpp.s
.PHONY : src/scheduled.cpp.s

src/server.o: src/server.cpp.o

.PHONY : src/server.o

# target to build an object file
src/server.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/server.cpp.o
.PHONY : src/server.cpp.o

src/server.i: src/server.cpp.i

.PHONY : src/server.i

# target to preprocess a source file
src/server.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/server.cpp.i
.PHONY : src/server.cpp.i

src/server.s: src/server.cpp.s

.PHONY : src/server.s

# target to generate assembly for a file
src/server.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/server.cpp.s
.PHONY : src/server.cpp.s

src/socket.o: src/socket.cpp.o

.PHONY : src/socket.o

# target to build an object file
src/socket.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/socket.cpp.o
.PHONY : src/socket.cpp.o

src/socket.i: src/socket.cpp.i

.PHONY : src/socket.i

# target to preprocess a source file
src/socket.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/socket.cpp.i
.PHONY : src/socket.cpp.i

src/socket.s: src/socket.cpp.s

.PHONY : src/socket.s

# target to generate assembly for a file
src/socket.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/src/socket.cpp.s
.PHONY : src/socket.cpp.s

test/test.o: test/test.cpp.o

.PHONY : test/test.o

# target to build an object file
test/test.cpp.o:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/test/test.cpp.o
.PHONY : test/test.cpp.o

test/test.i: test/test.cpp.i

.PHONY : test/test.i

# target to preprocess a source file
test/test.cpp.i:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/test/test.cpp.i
.PHONY : test/test.cpp.i

test/test.s: test/test.cpp.s

.PHONY : test/test.s

# target to generate assembly for a file
test/test.cpp.s:
	$(MAKE) -f CMakeFiles/drpc.dir/build.make CMakeFiles/drpc.dir/test/test.cpp.s
.PHONY : test/test.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... drpc"
	@echo "... edit_cache"
	@echo "... src/async_socket.o"
	@echo "... src/async_socket.i"
	@echo "... src/async_socket.s"
	@echo "... src/async_watcher.o"
	@echo "... src/async_watcher.i"
	@echo "... src/async_watcher.s"
	@echo "... src/channel.o"
	@echo "... src/channel.i"
	@echo "... src/channel.s"
	@echo "... src/event_loop.o"
	@echo "... src/event_loop.i"
	@echo "... src/event_loop.s"
	@echo "... src/event_loop_group.o"
	@echo "... src/event_loop_group.i"
	@echo "... src/event_loop_group.s"
	@echo "... src/ip_address.o"
	@echo "... src/ip_address.i"
	@echo "... src/ip_address.s"
	@echo "... src/listener.o"
	@echo "... src/listener.i"
	@echo "... src/listener.s"
	@echo "... src/scheduled.o"
	@echo "... src/scheduled.i"
	@echo "... src/scheduled.s"
	@echo "... src/server.o"
	@echo "... src/server.i"
	@echo "... src/server.s"
	@echo "... src/socket.o"
	@echo "... src/socket.i"
	@echo "... src/socket.s"
	@echo "... test/test.o"
	@echo "... test/test.i"
	@echo "... test/test.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

