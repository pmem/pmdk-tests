#
# Copyright 2018, Intel Corporation
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

if (NOT DEVELOPER_MODE)
	return()
endif ()

find_package(PythonInterp 3.4 QUIET)
if (NOT PYTHONINTERP_FOUND)
	message(WARNING "python3 >= 3.4 not found")
	return()
endif ()

execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/scripts/check_format.py --check-prerequisites
		RESULT_VARIABLE PREREQUISITES_CHECK_ERROR
		OUTPUT_QUIET
		ERROR_QUIET)

if ("${PREREQUISITES_CHECK_ERROR}" GREATER "0")
	message(WARNING "code-format.py prerequisites check failed - code style and formatting is disabled\nclang-format 3.9 is required")
	return()
endif ()

add_custom_target(checkers ALL)
add_custom_target(cppformat
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/scripts/check_format.py -irp ${CMAKE_SOURCE_DIR})
add_custom_target(license-check
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/scripts/check_license.py -p ${CMAKE_SOURCE_DIR})
add_custom_target(style-check
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/scripts/check_format.py -rp ${CMAKE_SOURCE_DIR})

add_dependencies(checkers license-check)
add_dependencies(checkers style-check)

# Visual Studio solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(checkers PROPERTIES FOLDER checkers)
set_target_properties(cppformat PROPERTIES FOLDER checkers)
set_target_properties(license-check PROPERTIES FOLDER checkers)
set_target_properties(style-check PROPERTIES FOLDER checkers)
