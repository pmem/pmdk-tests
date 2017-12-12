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

if (WIN32)
	set(VARS_TO_SET "")

	if (NOT DEFINED ENV{PMDKInclude})
		set(VARS_TO_SET "${VARS_TO_SET} PMDKInclude")
	endif ()
	if (NOT DEFINED ENV{PMDKDebug})
		set(VARS_TO_SET "${VARS_TO_SET} PMDKDebug")
	endif ()
	if (NOT DEFINED ENV{PMDKRelease})
		set(VARS_TO_SET "${VARS_TO_SET} PMDKRelease")
	endif ()

	if (NOT VARS_TO_SET STREQUAL "")
		message(FATAL_ERROR "Following environmental variables need to be set to valid directories: ${VARS_TO_SET}")
	endif ()

	# Windows path conversion
	string(REGEX REPLACE "/" "\\\\" BINDIR ${CMAKE_CURRENT_BINARY_DIR})

	# Empty target which will copy PMDK DLLs to build/ directory when being built
	add_custom_target(CopyPmdkDlls
		# Other projects are dependant on CopyPmdkDlls which means that these commands
		# will be launched before $(Configuration) directory is created
		COMMAND if not exist "${BINDIR}\\$(Configuration)" mkdir "${BINDIR}\\$(Configuration)"
		COMMAND copy $ENV{PMDKDebug}\\..\\$(Configuration)\\libpmemblk.dll "${BINDIR}\\$(Configuration)\\libpmemblk.dll" /Y
		COMMAND copy $ENV{PMDKDebug}\\..\\$(Configuration)\\libpmemlog.dll "${BINDIR}\\$(Configuration)\\libpmemlog.dll" /Y
		COMMAND copy $ENV{PMDKDebug}\\..\\$(Configuration)\\libpmemobj.dll "${BINDIR}\\$(Configuration)\\libpmemobj.dll" /Y
		COMMAND copy $ENV{PMDKDebug}\\..\\$(Configuration)\\libpmempool.dll "${BINDIR}\\$(Configuration)\\libpmempool.dll" /Y
	)
endif()
