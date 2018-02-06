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
Script includes functions to check completeness of documentation of
PMDK library.

Returns list of:
-missing macros in header files,
-missing functions in .so files,
-missing functions and macros in Generated catalog,
-functions and macros which are not linked with man page.
"""
"""
Essential argument:
-r -- the PMDK library root path

Optional argument:
-h, --help -- show help message and exit
"""

from subprocess import STDOUT, PIPE, run
from os import listdir, path, linesep
from argparse import ArgumentParser


def _extract_macros(pmdk_path):
    """
    Extract macros from header files
    """
    list_macros = []
    prefixes = ('POBJ_', 'OID_', 'TOID', 'TX_', 'DIRECT_', 'D_')
    includes_path = path.join(pmdk_path, 'src', 'include', 'libpmemobj')
    macro_in_next_line = False

    for header_file in listdir(includes_path):
        with open(path.join(includes_path, header_file), 'r') as f:
            for line in f:

                """
                After phrase 'static inline', occurs definition of macro
                """
                if line.startswith('static inline'):
                    macro_in_next_line = True
                    continue

                elif macro_in_next_line:
                    if line.startswith(prefixes):
                        macro_name = line[:line.index('(')]
                        list_macros.append(macro_name.lower())
                    macro_in_next_line = False
                    """
                    Line which:
                    - beginning from '#define'
                    - not includes words: '(id)', 'FLAG'
                    includes name of macros.
                    Macros name have to ended by '(' or '\'
                    Exceptions:
                    - TOID_NULL
                    - Macros which name begin from 'D_'
                    """
                elif line and line.startswith('#define') and\
                        line.split(' ')[1].startswith(prefixes):
                    macro_name = line.replace('\t', ' ').split(' ')[1]
                    if not ('(id)' in line or 'FLAG' in line):
                        if '(' in macro_name and not\
                                macro_name.startswith('TOID_NULL'):
                            macro_name = macro_name[:macro_name.index('(')]
                            list_macros.append(macro_name.lower())
                        elif '\\' in macro_name:
                            macro_name = macro_name[:macro_name.index('\\')]
                            list_macros.append(macro_name.lower())
                        elif macro_name.startswith('D_'):
                            list_macros.append(macro_name.lower())
    return list_macros


def _extract_functions_from_so_files(pmdk_path):
    """
    Return functions from .so files.
    """
    list_func_from_so_files = []
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')
    for elem in listdir(path_to_so_files):
        """
        Exception: libvmmalloc.so does not includes names of functions of PMDK
        library.
        """
        if elem.endswith('.so') and elem != 'libvmmalloc.so':
            process = run(
                'nm ' + elem + ' | grep " T "', cwd=path_to_so_files,
                shell=True, stdout=PIPE, stderr=STDOUT)
            out = process.stdout.decode('UTF-8')
            for line in out.split('\n'):
                if (line) and line.split(' ')[2].split('\n')[0] !=\
                        '_pobj_debug_notice':
                    list_func_from_so_files.append(
                        line.split(' ')[2].split('\n')[0])
    return list_func_from_so_files


def _extract_funcs_and_macros_from_gen(pmdk_path):
    """
    Return list of functions and macros from Generated catalog.
    """
    list_func_from_gen_catalog = []
    path_to_functions = path.join(pmdk_path, 'doc', 'generated')

    for function in listdir(path_to_functions):
        """
        File with extension '.3' have the same name like functions and macros
        of PMDK library.
        """
        if function.split('.')[0] and function != 'Makefile' and\
            function.split('.')[1] == '3'\
                and not function.startswith('pmemobj_action'):
            list_func_from_gen_catalog.append(function.split('.')[0])
    return list_func_from_gen_catalog


def _check_completness_macros_in_gen(pmdk_path):
    """
    Return list of macros, missing in Generated catalog.
    """
    list_macros = _extract_macros(pmdk_path)
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(pmdk_path)
    missing_macros = []
    for macro in list_macros:
        if not macro in list_func_from_gen_catalog:
            missing_macros.append(macro.lower())
    return missing_macros


def _check_completness_funcs_in_gen(pmdk_path):
    """
    Return list of functions, missing in Generated catalog.
    """
    list_func_from_so_files = _extract_functions_from_so_files(pmdk_path)
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(pmdk_path)
    missing_functions_in_gen_catalog = []
    for function in list_func_from_so_files:
        if not function in list_func_from_gen_catalog:
            missing_functions_in_gen_catalog.append(function)
    return missing_functions_in_gen_catalog


def _check_completness_extract_funcs_and_macros(pmdk_path):
    """
    Return list of macros and functions, missing in extract functions
    and macros.
    """
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(pmdk_path)
    list_macros = _extract_macros(pmdk_path)
    list_func_from_so_files = _extract_functions_from_so_files(pmdk_path)
    missing_functions_in_gen_catalog = []
    for function in list_func_from_gen_catalog:
        if not (function in list_macros or function in list_func_from_so_files):
            missing_functions_in_gen_catalog.append(function)
    return missing_functions_in_gen_catalog


def _check_linking_manpages(pmdk_path):
    """
    Return list of macros and functions, which are not linked with man page.
    """
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(pmdk_path)
    list_function_without_manpage = []
    for function in list_func_from_gen_catalog:
        process_find_in_manpage = run(
            'man ' + function, cwd=pmdk_path, shell=True, stdout=PIPE,
            stderr=STDOUT)
        if process_find_in_manpage.returncode:
            list_function_without_manpage.append(function)
    return list_function_without_manpage


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument('-r', '--pmdk-root',
                        required=True, help='Path to the library\
                        root directory.')
    args = parser.parse_args()
    pmdk_path = args.pmdk_root

    missing_mac = _check_completness_macros_in_gen(pmdk_path)
    msg_1 = 'List of missing macros in "Generated" catalog: '
    for mac in missing_mac:
        msg_1 = msg_1 + linesep + mac
    print(msg_1)

    missing_funcs = _check_completness_funcs_in_gen(pmdk_path)
    msg_2 = linesep + 'List of missing functions in "Generated" catalog: '
    for func in missing_funcs:
        msg_2 = msg_2 + linesep + func
    print(msg_2)

    missing_funcs_and_mac = _check_completness_extract_funcs_and_macros(
        pmdk_path)
    msg_3 = linesep + 'List of missing macros and functions: '
    for elem in missing_funcs_and_mac:
        msg_3 = msg_3 + linesep + elem
    print(msg_3)

    no_link_funcs_and_mac = _check_linking_manpages(pmdk_path)
    msg_4 = linesep + 'List of macros and functions without linked manpage: '
    for elem in no_link_funcs_and_mac:
        msg_4 = msg_4 + linesep + elem + linesep
    print(msg_4)


if __name__ == '__main__':
    main()
