#!usr/bin/env python3

from subprocess import STDOUT, PIPE, run
from os import listdir, path
import unittest
import sys
from argparse import ArgumentParser


def _extract_macros(PMDK_PATH):
    # extract macros from header files
    list_macros = []
    prefixes = ['POBJ_', 'OID_', 'TOID', 'TX_', 'DIRECT_', 'D_']
    include_file_path = path.join(PMDK_PATH, 'src', 'include', 'libpmemobj')
    macro_in_next_line = False

    for header_file in listdir(include_file_path):
        with open(path.join(include_file_path, header_file), 'r') as head_file:
            for line in head_file:

                if line.startswith('static inline'):
                    macro_in_next_line = True
                    continue

                elif macro_in_next_line:
                    if line.startswith(tuple(prefixes)):
                        macro_name = line[:line.index('(')]
                        list_macros.append(macro_name.lower())
                    macro_in_next_line = False

                elif line and line.startswith('#define') and\
                        line.split(' ')[1].startswith(tuple(prefixes)):
                    macro_name = line.replace('\t', ' ').split(' ')[1]
                    if not ('(id)' in line or 'FLAG' in line):
                        if '(' in macro_name and not macro_name.startswith('TOID_NULL'):
                            macro_name = macro_name[:macro_name.index('(')]
                            list_macros.append(macro_name.lower())
                        elif '\\' in macro_name:
                            macro_name = macro_name[:macro_name.index('\\')]
                            list_macros.append(macro_name.lower())
                        elif macro_name.startswith('D_'):
                            list_macros.append(macro_name.lower())
    return list_macros


def _extract_functions_from_so_files(PMDK_PATH):
    # extract functions from .so files
    list_func_from_so_files = []
    path_to_so_files = path.join(PMDK_PATH, 'src', 'nondebug')
    for so_file in listdir(path_to_so_files):
        if so_file.endswith('.so') and so_file != 'libvmmalloc.so':
            process = run(
                'nm ' + so_file + ' | grep " T "', cwd=path_to_so_files, shell=True,
                stdout=PIPE, stderr=STDOUT)
            out = process.stdout.decode('UTF-8')
            for line in out.split('\n'):
                if (line) and line.split(' ')[2].split('\n')[0] != '_pobj_debug_notice':
                    list_func_from_so_files.append(
                        line.split(' ')[2].split('\n')[0])
    return list_func_from_so_files


def _extract_funcs_and_macros_from_gen(PMDK_PATH):
    _list_func_from_gen_catalog = []
    path_to_functions = path.join(PMDK_PATH, 'doc', 'generated')

    for function in listdir(path_to_functions):
        if function.split('.')[0] and function != 'Makefile' and function.split('.')[1] == '3'\
                and not function.startswith('pmemobj_action'):
            _list_func_from_gen_catalog.append(function.split('.')[0])
    return _list_func_from_gen_catalog


def _check_completness_macros_in_gen(PMDK_PATH):
    list_macros = _extract_macros(PMDK_PATH)
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(PMDK_PATH)
    missing_macros = []
    for macro in list_macros:
        if not macro in list_func_from_gen_catalog:
            missing_macros.append(macro.lower())
    return missing_macros


def _check_completness_funcs_in_gen(PMDK_PATH):
    list_func_from_so_files = _extract_functions_from_so_files(PMDK_PATH)
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(PMDK_PATH)
    missing_functions_in_so_files = []
    for function in list_func_from_so_files:
        if not function in list_func_from_gen_catalog:
            missing_functions_in_so_files.append(function)
    return missing_functions_in_so_files


def _check_completness_extract_funcs_and_macros(PMDK_PATH):
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(PMDK_PATH)
    list_macros = _extract_macros(PMDK_PATH)
    list_func_from_so_files = _extract_functions_from_so_files(PMDK_PATH)
    missing_functions_in_gen_catalog = []
    for function in list_func_from_gen_catalog:
        if not (function in list_macros or function in list_func_from_so_files):
            missing_functions_in_gen_catalog.append(function)
    return missing_functions_in_gen_catalog


def _check_linking_manpages(PMDK_PATH):
    list_func_from_gen_catalog = _extract_funcs_and_macros_from_gen(PMDK_PATH)
    list_function_without_manpage = []
    for function in list_func_from_gen_catalog:
        process_find_in_manpage = run(
            'man ' + function, cwd=PMDK_PATH, shell=True, stdout=PIPE, stderr=STDOUT)
        if process_find_in_manpage.returncode:
            list_function_without_manpage.append(function)
    return list_function_without_manpage


if __name__ == '__main__':

    parser = ArgumentParser(
        description='Script includes functions checking correntness of PMDK documentation')
    parser.add_argument('-r', '--pmdk-root',
                        required=True, help='Path to main catalog of library.')
    args = parser.parse_args()
    PMDK_PATH = args.pmdk_root

    missing_mac = _check_completness_macros_in_gen(PMDK_PATH)
    msg_1 = 'List of missing macros in \"Generetaed\" catalog: '
    for mac in missing_mac:
        msg_1 = msg_1 + '\n' + mac
    print(msg_1)

    missing_funcs = _check_completness_funcs_in_gen(PMDK_PATH)
    msg_2 = '\nList of missing functions in \"Generetaed\" catalog: '
    for func in missing_funcs:
        msg_2 = msg_2 + '\n' + func
    print(msg_2)

    missing_funcs_and_mac = _check_completness_extract_funcs_and_macros(
        PMDK_PATH)
    msg_3 = '\nList of missing macros and functions: '
    for elem in missing_funcs_and_mac:
        msg_3 = msg_3 + '\n' + elem
    print(msg_3)

    no_link_funcs_and_mac = _check_linking_manpages(PMDK_PATH)
    msg_4 = '\nList of macros and functions without linked manpage: '
    for elem in no_link_funcs_and_mac:
        msg_4 = msg_4 + '\n' + elem + '\n'
    print(msg_4)
