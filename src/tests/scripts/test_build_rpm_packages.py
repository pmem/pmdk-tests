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
This module includes functions to check if all rpm packages in PMDK library,
are built and tests which use these functions.

Tests check:
-if all required rpm packages are built,
-if rpm packages for all names of libraries extracted from .so files and
 exceptions were checked.
"""
"""
Required arguments:
-r <PMDK_path>  the PMDK library root path.
-b <with_RPM>   the flag if rpmem and rpmemd packages should be built.
                Options: y (yes), n (no).
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
    Returns packages version and system architecture from names of catalogs from
    rpm directory.
    """
    rpm_path = path.join(pmdk_path, 'rpm')
    version = ''
    architecture = ''
    for elem in listdir(rpm_path):
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


def get_libraries_names_from_so_files(pmdk_path):
    """
    Returns names of libraries from .so files, and information which packages
    should be built for individual libraries.
    """
    libraries_from_so_files = dict()
    path_to_so_files = path.join(pmdk_path, 'src', 'nondebug')

    for elem in listdir(path_to_so_files):
        if elem.endswith('.so') and elem.startswith('lib'):
            library_name = elem.split('.')[0]
            libraries_from_so_files[library_name] =\
                PACKAGES_INFO(basic=True, devel=True, debug=True,
                              debuginfo=True, debug_debuginfo=True)
    return libraries_from_so_files


def get_names_of_packages(packages_info, with_rpm, is_pmdk_debuginfo):
    """
    Returns names of packages, which should be built.
    """
    packages = []
    if not is_pmdk_debuginfo:
        types = ['-', '-debug-', '-devel-',
                 '-debuginfo-', '-debug-debuginfo-']
    else:
        types = ['-', '-debug-', '-devel-']
    for elem in packages_info:
        i = 0
        # checks if rpmem and rpmemd packages should be built
        if with_rpm in ['n', 'no']:
            if elem in ['rpmemd', 'librpmem']:
                continue
        elif with_rpm in ['y', 'yes']:
            # does not skip creating names of packages for rpmemd and librpmem
            pass
        else:
            sys.exit('Wrong with_RPM argument.')
        for type_ in types:
            if packages_info[elem][i]:
                package_name = elem + type_ + PMDK_VERSION + '.' +\
                    SYSTEM_ARCHITECTURE + '.rpm'
                packages.append(package_name)
            i += 1
    return packages


def check_completness_of_packages(pmdk_path, with_rpm):
    """
    Checks if names of built rpm packages are the same as names of packages,
    which should be built and returns missing packages. Exceptions are taken
    into account.
    """
    built_packages = get_built_packages(pmdk_path)
    # checks if 'pmdk-debuginfo' package is built
    if_pmdk_debug_info_pack = False
    pmdk_debuginfo_pack_name =\
        'pmdk-debuginfo-' + PMDK_VERSION + '.' + SYSTEM_ARCHITECTURE + '.rpm'
    if pmdk_debuginfo_pack_name in built_packages:
        if_pmdk_debug_info_pack = True

    exceptions = dict()
    exceptions['rpmemd'] = PACKAGES_INFO(
        basic=True, devel=False, debug=False, debuginfo=True,
        debug_debuginfo=False)
    exceptions['pmempool'] = PACKAGES_INFO(
        basic=True, devel=False, debug=False, debuginfo=True,
        debug_debuginfo=False)
    exceptions['libpmemobj++'] = PACKAGES_INFO(
        basic=False, devel=False, debug=True, debuginfo=False,
        debug_debuginfo=False)

    exceptional_packages = get_names_of_packages(
        exceptions, with_rpm, if_pmdk_debug_info_pack)
    missing_exceptional_packages = [
        elem for elem in exceptional_packages if elem not in built_packages]
    libraries = get_libraries_names_from_so_files(pmdk_path)
    library_packages = get_names_of_packages(
        libraries, with_rpm, if_pmdk_debug_info_pack)
    missing_library_packages = [
        elem for elem in library_packages if elem not in built_packages]
    missing_packages = missing_library_packages + missing_exceptional_packages
    return missing_packages


def check_completness_of_libraries(pmdk_path):
    """
    Checks if names of functions from .so files are the same as names of
    functions extracted from built rpm packages and returns missing functions.
    Exceptions are taken into account.
    """
    exceptions = ['pmempool', 'libpmemobj++', 'rpmemd', 'pmdk']
    libraries = get_libraries_names_from_so_files(pmdk_path)
    rpm_packages_path = path.join(pmdk_path, 'rpm', SYSTEM_ARCHITECTURE)
    unchecked_libraries = []
    # looks for the name of library/exception in rpm package name
    library_name_pattern = r'[\s]*([a-zA-Z+]+)-'
    for elem in listdir(rpm_packages_path):
        library_name = re.search(library_name_pattern, elem).group(1)
        if library_name not in libraries.keys() and library_name not in\
                exceptions and library_name not in unchecked_libraries:
            unchecked_libraries.append(library_name)
    return unchecked_libraries


def add_argument(argument_option):
    """
    Adds new option to the command line.
    """
    index = sys.argv.index(argument_option)
    argument = 0
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

    def test_checks_completeness_of_built_rpm_packages(self):
        """
        Checks if all rpm packages are built.
        """
        missing_packages = check_completness_of_packages(pmdk_path, with_rpm)
        error_msg = linesep + 'List of missing packages:'
        for package in missing_packages:
            error_msg += linesep + package
        self.assertFalse(missing_packages, error_msg)

    def test_checks_completeness_of_name_of_libraries_and_exceptions(self):
        """
        Checks if names of functions from .so files and exceptions are the same
        as functions/exceptions extracted from the name of built rpm packages.
        """
        missing_elements = check_completness_of_libraries(pmdk_path)
        error_msg = linesep +\
            'List of names of elements, for which did not check packages:'
        for elem in missing_elements:
            error_msg += linesep + elem
        self.assertFalse(missing_elements, error_msg)


if __name__ == '__main__':
    path_argument = '-r'
    rpm_build_argument = '-b'
    if '-h' in sys.argv or '--help' in sys.argv:
        print(__doc__)
        unittest.main()
    elif path_argument in sys.argv and rpm_build_argument in sys.argv:
        pmdk_path = add_argument(path_argument)
        with_rpm = add_argument(rpm_build_argument)
        PMDK_VERSION, SYSTEM_ARCHITECTURE =\
            get_package_version_and_system_architecture(pmdk_path)
        if pmdk_path and with_rpm:
            unittest.main()
    else:
        print(__doc__)
        print(unittest.main.__doc__)
