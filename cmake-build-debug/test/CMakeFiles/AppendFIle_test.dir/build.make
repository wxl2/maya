# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /home/wxl/software/clion/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/wxl/software/clion/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wxl/date/code/maya

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wxl/date/code/maya/cmake-build-debug

# Include any dependencies generated for this target.
include test/CMakeFiles/AppendFIle_test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/AppendFIle_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/AppendFIle_test.dir/flags.make

test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o: test/CMakeFiles/AppendFIle_test.dir/flags.make
test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o: ../test/AppendFIle_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wxl/date/code/maya/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o"
	cd /home/wxl/date/code/maya/cmake-build-debug/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o -c /home/wxl/date/code/maya/test/AppendFIle_test.cpp

test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.i"
	cd /home/wxl/date/code/maya/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wxl/date/code/maya/test/AppendFIle_test.cpp > CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.i

test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.s"
	cd /home/wxl/date/code/maya/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wxl/date/code/maya/test/AppendFIle_test.cpp -o CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.s

# Object files for target AppendFIle_test
AppendFIle_test_OBJECTS = \
"CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o"

# External object files for target AppendFIle_test
AppendFIle_test_EXTERNAL_OBJECTS =

test/AppendFIle_test: test/CMakeFiles/AppendFIle_test.dir/AppendFIle_test.cpp.o
test/AppendFIle_test: test/CMakeFiles/AppendFIle_test.dir/build.make
test/AppendFIle_test: base/libbase.a
test/AppendFIle_test: test/CMakeFiles/AppendFIle_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wxl/date/code/maya/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable AppendFIle_test"
	cd /home/wxl/date/code/maya/cmake-build-debug/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/AppendFIle_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/AppendFIle_test.dir/build: test/AppendFIle_test

.PHONY : test/CMakeFiles/AppendFIle_test.dir/build

test/CMakeFiles/AppendFIle_test.dir/clean:
	cd /home/wxl/date/code/maya/cmake-build-debug/test && $(CMAKE_COMMAND) -P CMakeFiles/AppendFIle_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/AppendFIle_test.dir/clean

test/CMakeFiles/AppendFIle_test.dir/depend:
	cd /home/wxl/date/code/maya/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wxl/date/code/maya /home/wxl/date/code/maya/test /home/wxl/date/code/maya/cmake-build-debug /home/wxl/date/code/maya/cmake-build-debug/test /home/wxl/date/code/maya/cmake-build-debug/test/CMakeFiles/AppendFIle_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/AppendFIle_test.dir/depend

