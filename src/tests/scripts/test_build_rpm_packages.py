#!usr/bin/env python3
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

"""
This module includes tests that check if all rpm packages in PMDK library are
built correctly.

Tests check:
-if all required rpm packages are built,
-if built rpm packages are consistent with names of libraries read from
 .so files and other elements (tools and "PMDK").

Required arguments:
-r <PMDK_path>    the PMDK library root path.

Optional arguments:
--without_rpmem    the flag if rpmem and rpmemd packages should not be built.
"""

from os import listdir, path, linesep
from collections import namedtuple
import unittest
import sys
import re

PACKAGES_INFO = namedtuple('packages_info',
                           'basic devel debug debuginfo debug_debuginfo')
PMDK_VERSION = ''
SYSTEM_ARCHITECTURE = ''


def get_package_version_and_system_architecture(pmdk_path):
    """
    Returns packages version and system architecture from names of directories
    from rpm directory.
    """
    rpm_directory = path.join(pmdk_path, 'rpm')
    version = ''
    architecture = ''
    for elem in listdir(rpm_directory):
        if '.src.rpm' in elem:
            # looks for the version number of rpm package in rpm package name
            version = re.search(r'[\s]*pmdk-([\S]+).src.rpm', elem).group(1)
        else:
            architecture = elem
    return version, architecture


def get_built_packages(pmdk_path):
    """
    Returns built rpm packages from rpm directory.
    """
    packages_path = path.join(pmdk_path, 'rpm', SYSTEM_ARCHITECTURE)
    packages = listdir(packages_path)
    return packages


def get_libraries_names_from_so_files(pmdk_path, is_pmdk_debuginfo):
    """
    Returns names of libraries from .so files, and information which packages
    should be built for individual libraries.
    """
    libraries_from_so_files = dict()
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')

    for elem in listdir(path_to_so_files):
        if elem.endswith('.so') and elem.startswith('lib'):
            library_name = elem.split('.')[0]
            if is_pmdk_debuginfo:
                libraries_from_so_files[library_name] =\
                    PACKAGES_INFO(basic=True, devel=True, debug=True,
                                  debuginfo=False, debug_debuginfo=False)
            else:
                libraries_from_so_files[library_name] =\
                    PACKAGES_INFO(basic=True, devel=True, debug=True,
                                  debuginfo=True, debug_debuginfo=True)
    return libraries_from_so_files


def get_names_of_packages(packages_info, without_rpmem):
    """
    Returns names of packages, that should be built.
    """
    packages = []
    types = ['-', '-debug-', '-devel-', '-debuginfo-', '-debug-debuginfo-']
    for elem in packages_info:
        # checks if rpmem and rpmemd packages should be built
        # skips creating names of packages for rpmemd and librpmem
        if without_rpmem:
            if elem in ['rpmemd', 'librpmem']:
                continue
        sets_of_information = zip(packages_info[elem], types)
        for kit in sets_of_information:
            if kit[0]:
                package_name = elem + kit[1] + PMDK_VERSION + '.' +\
                    SYSTEM_ARCHITECTURE + '.rpm'
                packages.append(package_name)
    return packages


def check_existence_of_pmdk_debuginfo_package(pmdk_path, built_packages):
    """
    Checks if 'pmdk-debuginfo' package is built
    """
    is_pmdk_debuginfo_package = False
    pmdk_debuginfo_package_name =\
        'pmdk-debuginfo-' + PMDK_VERSION + '.' + SYSTEM_ARCHITECTURE + '.rpm'
    if pmdk_debuginfo_package_name in built_packages:
        is_pmdk_debuginfo_package = True
    return is_pmdk_debuginfo_package


def find_missing_packages(pmdk_path, without_rpmem):
    """
    Checks if names of built rpm packages are the same as names of packages,
    which should be built and returns missing packages. Tools are taken
    into account.
    """
    built_packages = get_built_packages(pmdk_path)
    is_pmdk_debuginfo =\
        check_existence_of_pmdk_debuginfo_package(pmdk_path, built_packages)
    tools = {
        'rpmemd': PACKAGES_INFO(basic=True, devel=False, debug=False,
                                debuginfo=True, debug_debuginfo=False),
        'pmempool': PACKAGES_INFO(basic=True, devel=False, debug=False,
                                  debuginfo=True, debug_debuginfo=False),
        'pmreorder': PACKAGES_INFO(basic=True, devel=False, debug=False,
                                   debuginfo=False, debug_debuginfo=False),
        'daxio': PACKAGES_INFO(basic=True, devel=False, debug=False,
                               debuginfo=True, debug_debuginfo=False)
    }
    tools_packages = get_names_of_packages(tools, without_rpmem)
    missing_tools_packages = [
        elem for elem in tools_packages if elem not in built_packages]
    libraries = get_libraries_names_from_so_files(pmdk_path, is_pmdk_debuginfo)
    library_packages = get_names_of_packages(libraries, without_rpmem)
    missing_library_packages = [
        elem for elem in library_packages if elem not in built_packages]
    missing_packages = missing_library_packages + missing_tools_packages
    return missing_packages


def find_missing_libraries_and_other_elements(pmdk_path):
    """
    Checks if names of functions from .so files are the same as names of
    functions extracted from built rpm packages and returns missing functions.
    Others rpm (tools and "PMDK") are taken into account.
    """
    others_rpm = ['pmempool', 'daxio', 'rpmemd', 'pmdk', 'pmreorder']
    built_packages = get_built_packages(pmdk_path)
    is_pmdk_debuginfo =\
        check_existence_of_pmdk_debuginfo_package(pmdk_path, built_packages)
    libraries = get_libraries_names_from_so_files(pmdk_path, is_pmdk_debuginfo)
    rpm_packages_path = path.join(pmdk_path, 'rpm', SYSTEM_ARCHITECTURE)
    missing_elements = []
    # looks for the name of library/others rpm in rpm package name
    library_name_pattern = r'[\s]*([a-zA-Z+]+)-'
    for elem in listdir(rpm_packages_path):
        library_name = re.search(library_name_pattern, elem).group(1)
        if library_name not in libraries.keys() and library_name not in\
                others_rpm and library_name not in missing_elements:
            missing_elements.append(library_name)
    return missing_elements


def parse_argument(argument_option):
    """
    Parses an option from the command line.
    """
    index = sys.argv.index(argument_option)
    try:
        argument = sys.argv[index+1]
    except IndexError:
        print('ERROR: Invalid argument!')
        print(__doc__)
        print(unittest.main.__doc__)
    else:
        sys.argv.pop(index)
        sys.argv.pop(index)
        return argument


class TestBuildRpmPackages(unittest.TestCase):

    def test_completeness_of_built_rpm_packages(self):
        """
        Checks if all rpm packages are built.
        """
        missing_packages =\
            find_missing_packages(pmdk_path, without_rpmem)
        error_msg = linesep + 'List of missing packages:'
        for package in missing_packages:
            error_msg += linesep + package
        self.assertFalse(missing_packages, error_msg)

    def test_completeness_of_name_of_libraries_and_others_rpm(self):
        """
        Checks if names of functions from .so files and other elements (tools
        and "PMDK") are the same as functions/other elements extracted from
        the name of built rpm packages.
        """
        missing_elements =\
            find_missing_libraries_and_other_elements(pmdk_path)
        error_msg = linesep +\
            'List of missing libraries and other elements (tools and "PMDK"):'
        for elem in missing_elements:
            error_msg += linesep + elem
        self.assertFalse(missing_elements, error_msg)


if __name__ == '__main__':
    path_argument = '-r'
    rpm_build_argument = '--without_rpmem'
    without_rpmem = False
    if '-h' in sys.argv or '--help' in sys.argv:
        print(__doc__)
        unittest.main()
    elif path_argument in sys.argv:
        pmdk_path = parse_argument(path_argument)
        if rpm_build_argument in sys.argv:
            without_rpmem = True
            index = sys.argv.index(rpm_build_argument)
            sys.argv.pop(index)
        if pmdk_path:
            PMDK_VERSION, SYSTEM_ARCHITECTURE =\
                get_package_version_and_system_architecture(pmdk_path)
            unittest.main()
    else:
        print(__doc__)
        print(unittest.main.__doc__)
