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
Script includes functions to check if documentation of PMDK library is built
correctly and tests which test these functions.

Tests check:
-if all macros from header files are included in the Generated directory,
-if all functions from .so files are included in the Generated directory,
-if all functions and macros from the Generated directory, are included
 in headers files and .so files,
-if all functions and macros have direct access to man page through use 
 command 'man function_name/macro_name'.
"""
"""
Required argument:
path -- the PMDK library root path (as the last argument).

Optional arguments are accessible in help (--help).
"""

import unittest
from subprocess import check_output, call, DEVNULL
from os import listdir, path, linesep
import re
import sys


def extract_macros(pmdk_path):
    """
    Function parses all headers from libpmemobj library, extracts all macros and 
    returns them in a list. There is a file 'exceptions.txt' in working
    directory, which includes all elements, which are not described in man page.
    """
    list_macros = []
    includes_path = path.join(pmdk_path, 'src', 'include', 'libpmemobj')
    macro_in_next_line = False
    exceptions_file_path = path.join(
        path.dirname(path.realpath(__file__)), 'exceptions.txt')
    with open(exceptions_file_path, 'r') as f:
        exceptions = [line.strip().lower() for line in f]
    # Definition of macro occurs after the phrases: 'static inline'
    # or '#define'
    for header_file in listdir(includes_path):
        with open(path.join(includes_path, header_file), 'r') as f:
            for line in f:
                if line.startswith('static inline'):
                    macro_in_next_line = True
                    continue

                elif macro_in_next_line:
                    expression_at_the_line_start = '[\s]*([a-zA-Z\_]+)[\\\(\s]+'
                    macro_name = re.search(expression_at_the_line_start, line)\
                        .group(1).lower()
                    if macro_name not in exceptions:
                        list_macros.append(macro_name)
                    macro_in_next_line = False

                elif re.search('#define', line):
                    expression_after_define_phrase =\
                        '[/s]*#define[\s]*([a-zA-Z\_]+)[\\\(\s]+'
                    macro_name = re.search(
                        expression_after_define_phrase, line).group(1).lower()
                    if macro_name not in exceptions:
                        list_macros.append(macro_name)
    return list_macros


def extract_functions_from_so_files(pmdk_path):
    """
    Function parses names of .so files, extracts names of functions and returns
    them in a list.
    """
    list_func_from_so_files = []
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')
    for elem in listdir(path_to_so_files):
        # Exception: libvmmalloc.so does not include names of functions of PMDK
        # library.
        if elem.endswith('.so') and elem != 'libvmmalloc.so':
            process = check_output(
                'nm ' + elem + ' | grep " T "', cwd=path_to_so_files,
                shell=True)
            out = process.decode('UTF-8')
            for line in out.split(linesep):
                if line:
                    name = line.split(' ')[2].strip()
                    # '_pobj_debug_notice' is an exception. It is a name of
                    # a function which is not described in man page.
                    if name != '_pobj_debug_notice':
                        list_func_from_so_files.append(name)
    return list_func_from_so_files


def extract_funcs_and_macros_from_gen(pmdk_path):
    """
    Function parses names of files from the Generated directory, extracts names
    from files with '.3'. extension (they includes names of functions and macros
    described in man page) and returns them in a list.
    """
    path_to_functions_and_macros = path.join(pmdk_path, 'doc', 'generated')
    # Files with extension '.3' have the same name like functions and macros
    # of PMDK library. 'pmemobj_action' is an exception. It is not a name of
    # function.
    list_funcs_and_macros_from_gen_directory = [elem.split('.')[0]
            for elem in listdir(path_to_functions_and_macros)
            if elem.endswith('.3') and not elem.startswith('pmemobj_action')]
    return list_funcs_and_macros_from_gen_directory


def check_completeness_of_macros_in_genereted_directory(pmdk_path):
    """
    Function checks if all macros from header files are included
    in the Generated directory based on macros returned by function
    'extract_macros()' and returns missing macros in a list.
    """
    list_macros = extract_macros(pmdk_path)
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    missing_macros = [macro for macro in list_macros if macro not in
                      list_func_from_gen_directory]
    return missing_macros


def check_completeness_of_funcs_in_genereted_directory(pmdk_path):
    """
    Function checks if all functions from .so files are included
    in the Generated directory based on functions returned by function
    'extract_functions_from_so_files()' and returns missing functions in a list.
    """
    list_func_from_so_files = extract_functions_from_so_files(pmdk_path)
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    missing_functions_in_gen_directory =\
        [function for function in list_func_from_so_files
         if function not in list_func_from_gen_directory]
    return missing_functions_in_gen_directory


def check_completeness_of_extracted_funcs_and_macros(pmdk_path):
    """
    Function checks if all functions and macros from the Generated directory,
    are included in headers files and .so files based on functions and macros
    returned by function 'extract_funcs_and_macros_from_gen()' and returns
    missing elements in a list.
    """
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    list_macros = extract_macros(pmdk_path)
    list_func_from_so_files = extract_functions_from_so_files(pmdk_path)
    missing_functions_and_macros_in_gen_directory = [
        item for item in list_func_from_gen_directory
        if not (item in list_macros or item in list_func_from_so_files)]
    return missing_functions_and_macros_in_gen_directory


def check_linking_manpages(pmdk_path):
    """
    Function checks if macros and functions from the Generated directory have
    direct access to man page through command 'man function_name/macro_name'
    and returns list with elements which are not linked with man page.
    """
    list_func_from_gen_directory = extract_funcs_and_macros_from_gen(pmdk_path)
    list_function_without_manpage = []
    for function in list_func_from_gen_directory:
        process_find_in_manpage = call(
            'man ' + function, shell=True, stdout=DEVNULL, stderr=DEVNULL)
        if process_find_in_manpage:
            list_function_without_manpage.append(function)
    return list_function_without_manpage


class TestDocumentation(unittest.TestCase):

    def test_checks_completeness_of_macros_in_generated_directory(self):
        """
        Test checks if all extracted macros from header files are
        in the Generated directory.
        """
        missing_mac =\
            check_completeness_of_macros_in_genereted_directory(pmdk_path)
        error_msg =\
            linesep + 'List of missing macros in the Generated directory:'
        for mac in missing_mac:
            error_msg += linesep + mac
        self.assertFalse(missing_mac, error_msg)

    def test_checks_completeness_of_functions_in_generated_directory(self):
        """
        Test checks if all extracted functions from .so files are
        in the Generated directory.
        """
        missing_funcs =\
            check_completeness_of_funcs_in_genereted_directory(pmdk_path)
        error_msg =\
            linesep + 'List of missing functions in the Generated directory:'
        for func in missing_funcs:
            error_msg += linesep + func
        self.assertFalse(missing_funcs, error_msg)

    def test_checks_completeness_of_extracted_functions_and_macros(self):
        """
        Test checks if all functions and macros from the Generated directory
        are extracted from headers files and .so files.
        """
        missing_funcs_and_mac =\
            check_completeness_of_extracted_funcs_and_macros(pmdk_path)
        error_msg = linesep + 'List of missing macros and functions:'
        for elem in missing_funcs_and_mac:
            error_msg += linesep + elem
        self.assertFalse(missing_funcs_and_mac, error_msg)

    def test_checks_if_functions_and_macros_are_linked_with_manpage(self):
        """
        Test checks if all functions and macros have direct access to man page 
        through use command 'man function_name/macro_name'.
        """
        no_link_funcs_and_mac = check_linking_manpages(pmdk_path)
        error_msg =\
            linesep + 'List of macros and functions without linked man page:'
        for elem in no_link_funcs_and_mac:
            error_msg += linesep + elem
        self.assertFalse(no_link_funcs_and_mac, error_msg)


if __name__ == '__main__':
    if '-h' in sys.argv or '--help' in sys.argv:
        unittest.main()
    else:
        pmdk_path = sys.argv[len(sys.argv) - 1]
        sys.argv.pop()
        unittest.main()
