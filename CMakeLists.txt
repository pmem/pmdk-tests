#
# Copyright 2017-2024, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# pmdk_tests - main CMakeLists.txt

cmake_minimum_required(VERSION 2.8.12)

project(pmdk_tests)

# ignore all the warnings
add_definitions(-w)

# check if 32-bit architecture
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "----Building as 64-bit is required. Please set build to 64-bit----")
	return()
endif ()

find_package(PkgConfig QUIET)
include(FindThreads)
include(functions.cmake) # Function definitions

option(DEVELOPER_MODE "enable developer checks" OFF)
option(PMDK_SYSTEM_HEADERS "always treat PMDK headers as system headers" ON)
include(style-format.cmake)

# Find and set GTest, PugiXML and PMDK libraries paths
download_gtest()
download_pugixml()

option(PMDK_INSTALL_PATH "PMDK installation path" "")
if (PKG_CONFIG_FOUND)
	pkg_check_modules(Libpmem REQUIRED libpmem)
	pkg_check_modules(Libpmemobj REQUIRED libpmemobj)
	pkg_check_modules(Libpmempool REQUIRED libpmempool)
	include_directories(${Libpmem_INCLUDE_DIRS}
			${Libpmemobj_INCLUDE_DIRS}
			${Libpmempool_INCLUDE_DIRS})

	if (PMDK_SYSTEM_HEADERS)
		set(CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -isystem ${Libpmem_INCLUDE_DIRS}")
		set(CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -isystem ${Libpmemobj_INCLUDE_DIRS}")
		set(CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -isystem ${Libpmempool_INCLUDE_DIRS}")
	endif ()

	link_directories(${Libpmem_LIBRARY_DIRS}
			${Libpmemobj_LIBRARY_DIRS}
			${Libpmempool_LIBRARY_DIRS})
else ()
	set(CMAKE_PREFIX_PATH "${PMDK_INSTALL_PATH}")
	find_library(Libpmem_LIBRARIES "pmem")
	find_library(Libpmemobj_LIBRARIES "pmemobj")
	find_library(Libpmempool_LIBRARIES "pmempool")

	find_path(PMDK_INCLUDE_DIR NAMES "libpmem.h" "libpmemobj.h" "libpmempool.h")
	include_directories(${PMDK_INCLUDE_DIR})
	if (PMDK_SYSTEM_HEADERS)
		set(CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -isystem ${PMDK_INCLUDE_DIR}")
	endif ()

endif ()

include_directories(src/utils)

# Set platform specific flags/includes/libs
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D_CRT_SECURE_NO_WARNINGS /DNVML_UTF8_API /DGTEST_LANG_CXX11")
	include_directories($ENV{PMDK_IncludePath})
else ()
	# Sets WORKAROUND_FLAGS variable which contains flags required to build with current configuration
	check_workaround_flags_required()

	# Disable RPATH so LD_LIBRARY_PATH can be used
	set(CMAKE_SKIP_RPATH ON)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Drestrict=__restrict__ -Werror -Wall -Wextra -pedantic-errors -Wpedantic -Wno-deprecated-declarations -std=c++11 ${WORKAROUND_FLAGS}")
endif ()

include(src/CMakeLists.txt)
