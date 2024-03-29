# Copyright (c) 2017-2023, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
#
# * Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
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

# US_REMOTE_AGENT
set(DIR ${CMAKE_CURRENT_LIST_DIR})
set(PREFIX_FILTER "")

file(GLOB_RECURSE us_remote_agent_SRC
	"${DIR}/*.h"
	"${DIR}/*.cc")

file(GLOB inject_utils
    "${DIR}/../inject_utils/*.h"
    "${DIR}/../inject_utils/*.cc")

add_executable(US_REMOTE_AGENT
	${us_remote_agent_SRC} ${inject_utils})

set_source_groups("${PREFIX_FILTER}" ${us_remote_agent_SRC})
include_directories(src/tests/RAS/ras_utils "${DIR}/../inject_utils")

target_link_libraries(US_REMOTE_AGENT RasUtils Utils)
add_dependencies(US_REMOTE_AGENT RasUtils Utils)
