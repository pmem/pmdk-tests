#
# Copyright 2018-2023, Intel Corporation
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

# RasUtils
set(DIR ${CMAKE_CURRENT_LIST_DIR})
set(PREFIX_FILTER "")

file(GLOB_RECURSE ras_utils_SRC
	"${DIR}/*.h"
	"${DIR}/*.cc")

set_source_groups("${PREFIX_FILTER}" ${ras_utils_SRC})
add_library(RasUtils STATIC ${ras_utils_SRC})
add_dependencies(RasUtils Utils)

pkg_check_modules(Libndctl REQUIRED libndctl)
include_directories(${Libndctl_INCLUDE_DIRS})
link_directories(${Libndctl_LIBRARY_DIRS})
pkg_check_modules(Libdaxctl REQUIRED libdaxctl)
include_directories(${Libdaxctl_INCLUDE_DIRS})
link_directories(${Libdaxctl_LIBRARY_DIRS})
target_link_libraries(RasUtils Utils ${Libpmemobj_LIBRARIES}
        ${Libdaxctl_LIBRARIES} ${Libndctl_LIBRARIES})
