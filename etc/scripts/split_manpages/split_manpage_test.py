#!usr/bin/env python3
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

"""
Tests check completness of PMDK library documentation. They use functions from
split_manpage_test.py.

Returns result of tests and messages errors.

Essential arguments:
path -- the PMDK library root path (as the last argument).

Optional arguments accessible in help (--help).
"""

from os import linesep
import unittest
import sys
import split_manpage as sm


class TestDocumentation(unittest.TestCase):

    def test_check_completness_of_macros_in_generated_catalog(self):
        """
        Test check if all extracted macros from header files are in "Generated"
        catalog.
        """
        missing_mac = sm._check_completness_macros_in_gen(pmdk_path)
        msg = linesep + 'List of missing macros in "Generated" catalog: '
        for mac in missing_mac:
            msg = msg + linesep + mac
        self.assertEqual(len(missing_mac), 0, msg)

    def test_check_completness_of_functions_in_generated_catalog(self):
        """
        Test check if all extracted functions from .so files are in "Generated"
        catalog.
        """
        missing_funcs = sm._check_completness_funcs_in_gen(pmdk_path)
        msg = linesep + 'List of missing functions in "Generated" catalog: '
        for func in missing_funcs:
            msg = msg + linesep + func
        self.assertEqual(len(missing_funcs), 0, msg)

    def test_check_completness_of_extracted_functions_and_macros(self):
        """
        Test check if all functions and macros from "Generated" catalog can be
        extracted
        from headers files and .so files.
        """
        missing_funcs_and_mac = sm._check_completness_extract_funcs_and_macros(
            pmdk_path)
        msg = linesep + 'List of missing macros and functions: '
        for elem in missing_funcs_and_mac:
            msg = msg + linesep + elem
        self.assertEqual(len(missing_funcs_and_mac), 0, msg)

    def test_check_if_functions_and_macros_are_linked_with_manpage_(self):
        """
        Test check if all functions and macros are split with man page.
        """
        no_link_funcs_and_mac = sm._check_linking_manpages(pmdk_path)
        msg = linesep + 'List of macros and functions without linked man page: '
        for elem in no_link_funcs_and_mac:
            msg = msg + linesep + elem
        self.assertEqual(len(no_link_funcs_and_mac), 0, msg)


if __name__ == '__main__':
    if '-h' in sys.argv or '--help' in sys.argv:
        unittest.main()
    else:
        pmdk_path = sys.argv[len(sys.argv) - 1]
        sys.argv.pop()
        unittest.main()
