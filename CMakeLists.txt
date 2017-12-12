#
# Copyright 2017, Intel Corporation
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
	set(Libpmemblk
			debug "$ENV{PMDKDebug}\\libpmemblk.lib"
			optimized "$ENV{PMDKRelease}\\libpmemblk.lib")

	set(Libpmemlog
			debug "$ENV{PMDKDebug}\\libpmemlog.lib"
			optimized "$ENV{PMDKRelease}\\libpmemlog.lib")

	set(Libpmemobj
			debug "$ENV{PMDKDebug}\\libpmemobj.lib"
			optimized "$ENV{PMDKRelease}\\libpmemobj.lib")

	set(Libpmempool
			debug "$ENV{PMDKDebug}\\libpmempool.lib"
			optimized "$ENV{PMDKRelease}\\libpmempool.lib")
else ()
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(Libpmemblk REQUIRED libpmemblk)
		pkg_check_modules(Libpmemlog REQUIRED libpmemlog)
		pkg_check_modules(Libpmemobj REQUIRED libpmemobj)
		pkg_check_modules(Libpmempool REQUIRED libpmempool)
	else ()
		find_library(Libpmemblk "pmemblk")
		find_library(Libpmemlog "pmemlog")
		find_library(Libpmemobj "pmemobj")
		find_library(Libpmempool "pmempool")
	endif ()
endif ()

include_directories(src/utils)

set(CMAKE_CXX_STANDARD 11)

# Set platform specific flags/includes/libs
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D_CRT_SECURE_NO_WARNINGS /DNVML_UTF8_API /DGTEST_LANG_CXX11")
	include_directories($ENV{PMDKInclude})
else ()
	# Disable RPATH so LD_LIBRARY_PATH can be used
	set(CMAKE_SKIP_RPATH ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Drestrict=__restrict__ -Werror -Wall -Wextra -pedantic-errors -Wpedantic")
endif ()

include(src/CMakeLists.txt) # Tests and Utils
include(windows_copy_pmdk_dlls.cmake) # CopyPmdkDlls
