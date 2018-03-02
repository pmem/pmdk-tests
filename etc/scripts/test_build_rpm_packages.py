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
Script includes functions to check if all rpm packages in PMDK library
are built and tests which test these functions.

Tests check:
-if all rpm packages are built,
-if rpm packages for all names of libraries and exceptions extracted
 from .so files are checking.
"""
"""
Required argument:
path -- the PMDK library root path (as the second argument from the end).
rpm_build -- the flag if rpmem and rpmemd packages are built 
             (as the last argument). Options: y (yes), n (no).

Optional arguments are accessible in help (--help).
"""

from os import listdir, path, linesep
import unittest
import sys
import re


def extract_number_of_package_and_system_version(pmdk_path):
    """
    Function extracts number of packages and name of system version from 'rpm'
    directory and return them.
    """
    rpm_path = path.join(pmdk_path, 'rpm')
    for elem in listdir(rpm_path):
        if re.search('.src.rpm', elem):
            name_without_extension = '[\s]*pmdk-([\S]+).src.rpm'
            version_number = re.search(name_without_extension, elem).group(1)
        else:
            sys_version = elem
    return version_number, sys_version


def extract_built_packages(pmdk_path):
    """
    Function extracts all built rpm packages based on files in 'rpm' directory
    and returns them in a list
    """
    version_number, sys_version =\
        extract_number_of_package_and_system_version(pmdk_path)
    packages_path = path.join(pmdk_path, 'rpm', sys_version)
    list_packages = [package for package in listdir(packages_path)]
    return list_packages


def extract_libraries_names_from_so_files(pmdk_path):
    """
    Function parses names of .so files, extracts names of libraries from them
    and returns result in a dictionary. Dictionary includes names of rpm
    packages and lists with boolean values to check if all types of packages are
    built. List defines types in the following way: [normal package,
    degub package, devel package, debuginfo package, debug-debuginfo package].
    Values:
    True - check if package are built,
    False - does not check if package are built.
    """
    list_of_libs_from_so_files = dict()
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')
    for elem in listdir(path_to_so_files):
        if elem.endswith('.so') and elem.startswith('lib'):
            library_name = elem.split('.')[0]
            list_of_libs_from_so_files[library_name] = [
                True, True, True, True, True]
    return list_of_libs_from_so_files


def create_list_with_names_of_packages(dict_lib, built_rpm, debuginfo):
    """
    Function based on given parameters creates list of names of rpm packages,
    which should be built.
    """
    list_packages = []
    version_number, sys_version = extract_number_of_package_and_system_version(
        pmdk_path)
    if not debuginfo:
        types = ['-', '-debug-', '-devel-', '-debuginfo-', '-debug-debuginfo-']
    else:
        types = ['-', '-debug-', '-devel-']

    for elem in dict_lib:
        if built_rpm in ['n', 'no']:
            if elem in ['rpmemd', 'librpmem']:
                continue
        elif built_rpm in ['y', 'yes']:
            pass
        else:
            sys.exit('Wrong rpm_build argument.')
        zipped_name_and_condition_types = zip(dict_lib[elem], types)
        for item in zipped_name_and_condition_types:
            if item[0]:
                package_name = elem + \
                    item[1] + version_number + '.' + sys_version + '.rpm'
                list_packages.append(package_name)
    return list_packages


def check_completness_of_packages(pmdk_path, with_rpm):
    """
    Function checks if functions returned by function
    'extract_built_packages()' overlap with functions retured by function
    'create_list_with_names_of_packages()'.
    """
    version_number, sys_version =\
        extract_number_of_package_and_system_version(pmdk_path)
    built_packages = extract_built_packages(pmdk_path)

    # check if 'pmdk-debuginfo package' are built
    pmdk_debubinfo_pack_name =\
        'pmdk-debuginfo-' + version_number + '.' + sys_version + '.rpm'
    if pmdk_debubinfo_pack_name in built_packages:
        if_pmdk_debub_info_pack = True
    else:
        if_pmdk_debub_info_pack = False

    no_libraries = {
        'rpmemd': [True, False, False, True, False],
        'pmempool': [True, False, False, True, False],
        'libpmemobj++': [False, False, True, False, False]}
    no_lib_packages = create_list_with_names_of_packages(
        no_libraries, with_rpm, if_pmdk_debub_info_pack)
    missing_packages_no_lib = [
        elem for elem in no_lib_packages if elem not in built_packages]

    libraries = extract_libraries_names_from_so_files(pmdk_path)
    lib_packages = create_list_with_names_of_packages(
        libraries, with_rpm, if_pmdk_debub_info_pack)
    missing_packages_lib = [
        elem for elem in lib_packages if elem not in built_packages]

    missing_packages = missing_packages_lib + missing_packages_no_lib

    return missing_packages


def check_completness_of_libraries(pmdk_path):
    """
    Function checks if names of functions from .so files overlap with functions
    extracted from name of built rpm packages. Function takes account
    exceptions.
    """
    exceptions = ['pmempool', 'libpmemobj++', 'rpmemd', 'pmdk']
    libraries = list(extract_libraries_names_from_so_files(pmdk_path).keys())
    version_number, sys_version =\
        extract_number_of_package_and_system_version(pmdk_path)
    rpm_packages_path = path.join(pmdk_path, 'rpm', sys_version)
    libs_and_exceps = exceptions + libraries
    beginning_file_name = '[\s]*([a-zA-Z+]+)\-'
    list_unchecked_libraries = [
        re.search(beginning_file_name, elem).group(1)
        for elem in listdir(rpm_packages_path)
        if re.search(beginning_file_name, elem).group(1)
        not in libs_and_exceps]
    return list_unchecked_libraries


class TestBuildRpmPackages(unittest.TestCase):

    def test_checks_completeness_of_built_rpm_packages(self):
        """
        Test checks if functions returned by function
        'extract_built_packages()' overlap with functions returned by function
        'create_list_with_names_pacages()'.
        """
        missing_pack = check_completness_of_packages(pmdk_path, with_rpm)
        error_msg = linesep + 'List of missing packages:'
        for elem in missing_pack:
            error_msg += linesep + elem
        self.assertEqual(len(missing_pack), 0, error_msg)

    def test_checks_completeness_of_name_of_libraries_and_exceptions(self):
        """
        Test checks if names of functions from .so files overlap with functions
        extracted from name of built rpm packages. Function takes account
        exceptions.
        """
        missing_names = check_completness_of_libraries(pmdk_path)
        error_msg = linesep + 'List of names of elements, for which\
                               did not checked packages:'
        for elem in missing_names:
            error_msg += linesep + elem
        self.assertEqual(len(missing_names), 0, error_msg)


if __name__ == '__main__':
    if '-h' in sys.argv or '--help' in sys.argv:
        unittest.main()
    else:
        pmdk_path = sys.argv[len(sys.argv) - 2]
        with_rpm = sys.argv[len(sys.argv) - 1]
        sys.argv.pop()
        sys.argv.pop()
        unittest.main()
