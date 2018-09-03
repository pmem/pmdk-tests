# Copyright (c) 2017-2018, Intel Corporation
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

include(${CMAKE_CURRENT_LIST_DIR}/pmempools/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/pmemobj/CMakeLists.txt)

if (NOT WIN32)
		pkg_check_modules(Libndctl QUIET libndctl)
		pkg_check_modules(Libdaxctl QUIET libdaxctl)
		if (Libndctl_FOUND AND Libdaxctl_FOUND)
			include(${CMAKE_CURRENT_LIST_DIR}/ras/CMakeLists.txt)
		else ()
			if (NOT Libndctl_FOUND AND NOT Libdaxctl_FOUND)
				message(WARNING "Cannot find libndctl and libdaxctl libraries. Skip building RAS tests.")
			elseif (NOT Libndctl_FOUND)
				message(WARNING "Cannot find libndctl library. Skip building RAS tests.")
			else ()
				message(WARNING "Cannot find libdaxctl library. Skip building RAS tests.")
			endif()
		endif ()
else ()
		include(${CMAKE_CURRENT_LIST_DIR}/ras/CMakeLists.txt)
endif()
