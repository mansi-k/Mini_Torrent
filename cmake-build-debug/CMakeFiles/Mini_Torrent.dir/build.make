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
CMAKE_COMMAND = /snap/clion/129/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/129/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Mini_Torrent.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Mini_Torrent.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Mini_Torrent.dir/flags.make

CMakeFiles/Mini_Torrent.dir/main.cpp.o: CMakeFiles/Mini_Torrent.dir/flags.make
CMakeFiles/Mini_Torrent.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Mini_Torrent.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Mini_Torrent.dir/main.cpp.o -c /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/main.cpp

CMakeFiles/Mini_Torrent.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Mini_Torrent.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/main.cpp > CMakeFiles/Mini_Torrent.dir/main.cpp.i

CMakeFiles/Mini_Torrent.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Mini_Torrent.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/main.cpp -o CMakeFiles/Mini_Torrent.dir/main.cpp.s

# Object files for target Mini_Torrent
Mini_Torrent_OBJECTS = \
"CMakeFiles/Mini_Torrent.dir/main.cpp.o"

# External object files for target Mini_Torrent
Mini_Torrent_EXTERNAL_OBJECTS =

Mini_Torrent: CMakeFiles/Mini_Torrent.dir/main.cpp.o
Mini_Torrent: CMakeFiles/Mini_Torrent.dir/build.make
Mini_Torrent: CMakeFiles/Mini_Torrent.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Mini_Torrent"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Mini_Torrent.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Mini_Torrent.dir/build: Mini_Torrent

.PHONY : CMakeFiles/Mini_Torrent.dir/build

CMakeFiles/Mini_Torrent.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Mini_Torrent.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Mini_Torrent.dir/clean

CMakeFiles/Mini_Torrent.dir/depend:
	cd /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug /home/mansi/IIITH/AOS/Assgn2/Mini_Torrent/cmake-build-debug/CMakeFiles/Mini_Torrent.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Mini_Torrent.dir/depend

