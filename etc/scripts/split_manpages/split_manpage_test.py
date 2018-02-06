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
Script includes functions to check completeness of documentation of PMDK
library and tests which test completeness of documentation of PMDK library.

Tests check:
-completeness of macros from header files,
-completeness of functions from .so files,
-completeness of functions and macros in Generated directory,
-if all functions and macros have direct access to man page through use 
 command 'man function_name/macro_name'.
"""
"""
Required argument:
path -- the PMDK library root path (as the last argument).

Optional arguments accessible in help (--help).
"""

import unittest
from subprocess import STDOUT, PIPE, run
from os import listdir, path, linesep, getcwd
from argparse import ArgumentParser
from re import search
import sys


def extract_macros(pmdk_path):
    """
    Function parses all headers from libpmemobj library, extracts all macros and 
    returns them in a list. In working directory there are file
    'exceptions.txt', includes all elements, which are not describe in man page.
    """
    list_macros = []
    exceptions = []
    includes_path = path.join(pmdk_path, 'src', 'include', 'libpmemobj')
    macro_in_next_line = False
    exceptions_file_directory = path.join(
        path.dirname(path.realpath(__file__)), 'exceptions.txt')
    with open(exceptions_file_directory, 'r') as f:
        for line in f:
            exceptions.append(line.strip().lower())
    # Definition of macro occurs after the phrases: 'static inline'
    # and '#define'
    for header_file in listdir(includes_path):
        with open(path.join(includes_path, header_file), 'r') as f:
            for line in f:
                if line.startswith('static inline'):
                    macro_in_next_line = True
                    continue

                elif macro_in_next_line:
                    macro_name = search('[\s]*([a-zA-Z\_]+)[\\\(\s]+', line)\
                        .group(1).lower()
                    if macro_name not in exceptions:
                        list_macros.append(macro_name)
                    macro_in_next_line = False

                elif search('#define', line):
                    macro_name = search(
                        '[/s]*#define[\s]*([a-zA-Z\_]+)[\\\(\s]+', line).\
                        group(1).lower()
                    if macro_name not in exceptions:
                        list_macros.append(macro_name)
    return list_macros


def extract_functions_from_so_files(pmdk_path):
    """
    Function parses name of .so files, extracts names of functions and returns 
    them in a list.
    """
    list_func_from_so_files = []
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')
    for elem in listdir(path_to_so_files):
        # Exception: libvmmalloc.so does not include names of functions of PMDK
        # library.
        if elem.endswith('.so') and elem != 'libvmmalloc.so':
            process = run(
                'nm ' + elem + ' | grep " T "', cwd=path_to_so_files,
                shell=True, stdout=PIPE, stderr=STDOUT)
            out = process.stdout.decode('UTF-8')
            for line in out.split(linesep):
                if line:
                    name = line.split(' ')[2].strip()
                    if name != '_pobj_debug_notice':
                        list_func_from_so_files.append(name)
    return list_func_from_so_files


def extract_funcs_and_macros_from_gen(pmdk_path):
    """
    Function parse names of files from Generated directory, extract names from
    files with '.3. extension (they includes names of functions and macros
    described in man page) and returns them in a list.
    """
    list_func_from_gen_directory = []
    path_to_functions = path.join(pmdk_path, 'doc', 'generated')

    for function in listdir(path_to_functions):
        # Files with extension '.3' have the same name like functions and macros
        # of PMDK library.
        if function.split('.')[0] and function != 'Makefile' and\
            function.split('.')[1] == '3'\
                and not function.startswith('pmemobj_action'):
            list_func_from_gen_directory.append(function.split('.')[0])
    return list_func_from_gen_directory


def check_completeness_macros_in_gen(pmdk_path):
    """
    Function checks completeness of macros in Generated directory based on
    macros returned by function 'extract_macros()' and returns missing macros 
    in a list.
    """
    list_macros = extract_macros(pmdk_path)
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    missing_macros = [macro for macro in list_macros if macro not in
                      list_func_from_gen_directory]
    return missing_macros


def check_completeness_funcs_in_gen(pmdk_path):
    """
    Function checks completeness of functions in Generated directory based on
    functions returned by function 'extract_functions_from_so_files()' and 
    returns missing functions in a list.
    """
    list_func_from_so_files = extract_functions_from_so_files(pmdk_path)
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    missing_functions_in_gen_directory =\
        [function for function in list_func_from_so_files
         if function not in list_func_from_gen_directory]
    return missing_functions_in_gen_directory


def check_completeness_extract_funcs_and_macros(pmdk_path):
    """
    Function checks completeness of functions and macros extracted from header 
    files and .so files based on functions and macros returned by function 
    'extract_funcs_and_macros_from_gen() and returns missing elements in a list.
    """
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    list_macros = extract_macros(pmdk_path)
    list_func_from_so_files = extract_functions_from_so_files(pmdk_path)
    missing_functions_and_macros_in_gen_directory =\
        [item for item in list_func_from_gen_directory
         if not (item in list_macros or item in list_func_from_so_files)]
    return missing_functions_and_macros_in_gen_directory


def check_linking_manpages(pmdk_path):
    """
    Function checks if macros and functions from Generated directory have direct
    access to man page through by use command 'man function_name/macro_name' and
    returns list with elements which are not linked with man page.
    """
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    list_function_without_manpage = []
    for function in list_func_from_gen_directory:
        process_find_in_manpage = run(
            'man ' + function, cwd=pmdk_path, shell=True, stdout=PIPE,
            stderr=STDOUT)
        if process_find_in_manpage.returncode:
            list_function_without_manpage.append(function)
    return list_function_without_manpage


class TestDocumentation(unittest.TestCase):

    def test_checks_completeness_of_macros_in_generated_directory(self):
        """
        Test checks if all extracted macros from header files are in "Generated"
        directory.
        """
        missing_mac = check_completeness_macros_in_gen(pmdk_path)
        error_msg =\
            linesep + 'List of missing macros in "Generated" directory: '
        for mac in missing_mac:
            error_msg += linesep + mac
        self.assertEqual(len(missing_mac), 0, error_msg)

    def test_checks_completeness_of_functions_in_generated_directory(self):
        """
        Test checks if all extracted functions from .so files are in "Generated"
        directory.
        """
        missing_funcs = check_completeness_funcs_in_gen(pmdk_path)
        error_msg =\
            linesep + 'List of missing functions in "Generated" directory: '
        for func in missing_funcs:
            error_msg += linesep + func
        self.assertEqual(len(missing_funcs), 0, error_msg)

    def test_checks_completeness_of_extracted_functions_and_macros(self):
        """
        Test checks if all functions and macros from "Generated" directory are
        extracted from headers files and .so files.
        """
        missing_funcs_and_mac = check_completeness_extract_funcs_and_macros(
            pmdk_path)
        error_msg = linesep + 'List of missing macros and functions: '
        for elem in missing_funcs_and_mac:
            error_msg += linesep + elem
        self.assertEqual(len(missing_funcs_and_mac), 0, error_msg)

    def test_checks_if_functions_and_macros_are_linked_with_manpage(self):
        """
        Test checks if all functions and macros have direct access to man page 
        through use command 'man function_name/macro_name'
        """
        no_link_funcs_and_mac = check_linking_manpages(pmdk_path)
        error_msg =\
            linesep + 'List of macros and functions without linked man page: '
        for elem in no_link_funcs_and_mac:
            error_msg += linesep + elem
        self.assertEqual(len(no_link_funcs_and_mac), 0, error_msg)


if __name__ == '__main__':
    if '-h' in sys.argv or '--help' in sys.argv:
        unittest.main()
    else:
        pmdk_path = sys.argv[len(sys.argv) - 1]
        sys.argv.pop()
        unittest.main()
