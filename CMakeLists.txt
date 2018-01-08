#
# Copyright 2017-2018, Intel Corporation
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

if (WIN32)
	cmake_minimum_required(VERSION 3.1)
else ()
	cmake_minimum_required(VERSION 2.8.12)
endif()

project(pmdk_tests)

# check if 32-bit architecture
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "----Building as 64-bit is required. Please set build to 64-bit----")
	return()
endif ()

find_package(PkgConfig QUIET)
include(FindThreads)
include(functions.cmake) # Function definitions

# Find and set GTest, PugiXML and PMDK libraries paths
download_gtest()
download_pugixml()

if (WIN32)
	set(VARS_TO_SET "")

	if (NOT DEFINED ENV{PMDK_ExecutablePath})
		set(VARS_TO_SET "${VARS_TO_SET} PMDK_ExecutablePath")
	endif ()
	if (NOT DEFINED ENV{PMDK_ExecutablePath_Dbg})
		set(VARS_TO_SET "${VARS_TO_SET} PMDK_ExecutablePath_Dbg")
	endif ()
	if (NOT DEFINED ENV{PMDK_IncludePath})
		set(VARS_TO_SET "${VARS_TO_SET} PMDK_IncludePath")
	endif ()
	if (NOT DEFINED ENV{PMDK_LibraryPath})
		set(VARS_TO_SET "${VARS_TO_SET} PMDK_LibraryPath")
	endif ()
	if (NOT DEFINED ENV{PMDK_LibraryPath_Dbg})
		set(VARS_TO_SET "${VARS_TO_SET} PMDK_LibraryPath_Dbg")
	endif ()

	if (NOT VARS_TO_SET STREQUAL "")
		message(FATAL_ERROR "Following environment variables need to be set to valid directories: ${VARS_TO_SET}")
	endif ()

	set(Libpmemblk
			debug "$ENV{PMDK_LibraryPath_Dbg}\\libpmemblk.lib"
			optimized "$ENV{PMDK_LibraryPath}\\libpmemblk.lib")

	set(Libpmemlog
			debug "$ENV{PMDK_LibraryPath_Dbg}\\libpmemlog.lib"
			optimized "$ENV{PMDK_LibraryPath}\\libpmemlog.lib")

	set(Libpmemobj
			debug "$ENV{PMDK_LibraryPath_Dbg}\\libpmemobj.lib"
			optimized "$ENV{PMDK_LibraryPath}\\libpmemobj.lib")

	set(Libpmempool
			debug "$ENV{PMDK_LibraryPath_Dbg}\\libpmempool.lib"
			optimized "$ENV{PMDK_LibraryPath}\\libpmempool.lib")
else ()
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(Libpmemblk REQUIRED libpmemblk)
		pkg_check_modules(Libpmemlog REQUIRED libpmemlog)
		pkg_check_modules(Libpmemobj REQUIRED libpmemobj)
		pkg_check_modules(Libpmempool REQUIRED libpmempool)
		include_directories(${Libpmemblk_INCLUDE_DIRS} ${Libpmemlog_INCLUDE_DIRS} ${Libpmemobj_INCLUDE_DIRS} ${Libpmempool_INCLUDE_DIRS})
		link_directories(${Libpmemblk_LIBRARY_DIRS} ${Libpmemlog_LIBRARY_DIRS} ${Libpmemobj_LIBRARY_DIRS} ${Libpmempool_LIBRARY_DIRS})
	else ()
		find_library(Libpmemblk "pmemblk")
		find_library(Libpmemlog "pmemlog")
		find_library(Libpmemobj "pmemobj")
		find_library(Libpmempool "pmempool")
		include_directories(${CMAKE_PREFIX_PATH}/include)
		include_directories(${CMAKE_PREFIX_PATH}/lib)
	endif ()
endif ()

include_directories(src/utils)

# Set platform specific flags/includes/libs
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D_CRT_SECURE_NO_WARNINGS /DNVML_UTF8_API /DGTEST_LANG_CXX11")
	include_directories($ENV{PMDK_IncludePath})
else ()
	# Disable RPATH so LD_LIBRARY_PATH can be used
	set(CMAKE_SKIP_RPATH ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Drestrict=__restrict__ -Werror -Wall -Wextra -pedantic-errors -Wpedantic -std=c++11")
endif ()

include(src/CMakeLists.txt)
