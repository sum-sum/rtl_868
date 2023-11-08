# rtl_868 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# rtl_868 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.6)
project(rtl868 C)
set (rtl868_VERSION_MAJOR 1)
set (rtl868_VERSION_MINOR 0)

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")


OPTION(RTL_FULL_STATIC_BUILD "Build rtl_868 fully static." OFF)

########################################################################
# Compiler specific setup
########################################################################
if(CMAKE_COMPILER_IS_GNUCC AND NOT WIN32)
    ADD_DEFINITIONS(-Wall)
    ADD_DEFINITIONS(-Wextra)
    ADD_DEFINITIONS(-Wno-unused)
    ADD_DEFINITIONS(-Wsign-compare)
    ADD_DEFINITIONS(-g3 -O0)
    ADD_DEFINITIONS(-std=gnu99)
    #http://gcc.gnu.org/wiki/Visibility
    add_definitions(-fvisibility=hidden)
endif()

if(RTL_FULL_STATIC_BUILD)
    if (WIN32)
        if(MINGW)
            # Special MINGW stuff here
            # see https://cmake.org/pipermail/cmake/2012-September/051970.html
            # see http://stackoverflow.com/questions/13768515/how-to-do-static-linking-of-libwinpthread-1-dll-in-mingw
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
            set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static -static-libgcc -s")
            set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static -static-libgcc -static-libstdc++ -s")
        endif()
    endif()
endif()

if(WIN32)
    add_definitions( -DWIN32 -D_WIN32 )
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions( -DNDEBUG )
endif()


########################################################################
# Build utility
########################################################################
add_executable(rtl_868
	ws300.c
	transmission.c
	nrz_decode.c
	main.c
	logging.c
	tx29.c
	tools.c
	data_logger.c
)

#target_link_libraries(rtl_868
#	${CMAKE_THREAD_LIBS_INIT}
#)


set(INSTALL_TARGETS rtl_868)
if(UNIX)
target_link_libraries(rtl_868 rt m)
endif()

########################################################################
# Install built library files & utilities
########################################################################
install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION bin              # .dll file
)
