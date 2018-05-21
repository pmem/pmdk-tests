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
This module includes tests for PMDK rpm installation.

Tests check:
-compatibility of the version of installed rpm packages from PMDK library with
 the current version of PMDK library,
-if all rpm packages from PMDK library are installed.

Required arguments:
-r <PMDK_path>    the PMDK library root path.
"""

from os import listdir, path, linesep, walk
from subprocess import check_output
import unittest
import sys
import re

NO_PKG_CONFIGS = ('pmdk', 'pmempool')
PMDK_VERSION = ''
SYSTEM_ARCHITECTURE = ''
PMDK_PATH = ''


def get_package_version_and_system_architecture():
    """
    Returns packages version and system architecture from names of directories
    from rpm directory.
    """
    rpm_directory = path.join(PMDK_PATH, 'rpm')
    global PMDK_VERSION
    global SYSTEM_ARCHITECTURE
    for elem in listdir(rpm_directory):
        if '.src.rpm' in elem:
            # looks for the version number of rpm package in rpm package name
            PMDK_VERSION = re.search(
                r'[\s]*pmdk-([\S]+).src.rpm', elem).group(1)
        else:
            SYSTEM_ARCHITECTURE = elem


def get_libraries_names():
    """
    Returns names of elements, for which are installed packages from PMDK
    library.
    """
    rpm_packages_path = path.join(PMDK_PATH, 'rpm', SYSTEM_ARCHITECTURE)
    libraries_names = [elem.split('-')[0] for elem in listdir(rpm_packages_path)
                       if PMDK_VERSION in elem]
    return set(libraries_names)


def get_missing_headers():
    """
    Returns names of missing 'libpmemobj++' header files.
    """
    no_headers = {'README.md'}
    source_headers = []
    installed_headers = []
    for _, _, files in walk('/usr/include/libpmemobj++/'):
        installed_headers += files
    for _, _, files in walk(
            path.join(PMDK_PATH, 'src/include/libpmemobj++/')):
        source_headers += files
    diff = (set(source_headers) ^ set(installed_headers)) - no_headers
    return diff

def condition_if_no_action(elem):
    if elem == 'pmempool' and elem in listdir('/usr/bin/'):
        return True
    elif elem == "pmdk" or elem + '.so' in listdir('/usr/lib64/'):
        return True
    elif elem == "pmdk" or elem + '.so' in listdir('/usr/lib64/'):
        return True
    return False

def get_not_installed_rpm_packages():
    """
    Returns names of rpm packages from PMDK library, which are not installed.
    """
    elements = get_libraries_names()
    not_installed_packages = []
    missing_headers = set()
    for elem in elements:
        if condition_if_no_action(elem):
            pass
        elif elem == "libpmemobj++":
            missing_headers =\
                get_missing_headers()
            if missing_headers:
                not_installed_packages.append(elem)
        else:
            not_installed_packages.append(elem)
    return not_installed_packages


def get_incompatible_packages():
    """
    Returns names of rpm packages from PMDK library, which are not compatible
    with the current version of PMDK library.
    """
    pkgconfig_directory = '/usr/lib64/pkgconfig/'
    incompatibe_packages = []
    libraries = get_libraries_names() - set(NO_PKG_CONFIGS)
    for library in libraries:
        with open(pkgconfig_directory + library + '.pc') as f:
            out = f.readlines()
        for line in out:
            if 'version=' in line:
                version = line.split('=')[1].strip(linesep)
        if not version in PMDK_VERSION.replace('~', '-'):
            incompatibe_packages.append(library)
    return incompatibe_packages


class TestBuildRpmPackages(unittest.TestCase):

    def test_compatibility_of_version_of_installed_rpm_packages(self):
        """
        Checks if the version of installed rpm packages is correct.
        """
        incompatible_packages = get_incompatible_packages()
        error_msg = linesep + 'List of incompatible packages: '
        for package in incompatible_packages:
            error_msg += linesep + package
        self.assertFalse(incompatible_packages, error_msg)

    def test_correctness_of_installed_rpm_packages(self):
        """
        Checks if all rpm packages from PMDK library are installed.
        """
        not_installed_packages = get_not_installed_rpm_packages()
        missing_headers = get_missing_headers()
        error_msg = linesep + 'List of not installed packages: '
        for package in not_installed_packages:
            error_msg += linesep + package
        error_msg += linesep + 'List of missing libpmemobj++ headers: '
        for header in missing_headers:
            error_msg += linesep + header
        self.assertFalse(not_installed_packages, error_msg)


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


if __name__ == '__main__':
    if '-h' in sys.argv or '--help' in sys.argv:
        print(__doc__)
        unittest.main()
    elif '-r' in sys.argv:
        PMDK_PATH = parse_argument('-r')
        get_package_version_and_system_architecture()
        if PMDK_VERSION == '' or SYSTEM_ARCHITECTURE == '':
            sys.exit("FATAL ERROR: command 'make rpm' was not done correctly")
        unittest.main()
    else:
        print(__doc__)
        print(unittest.main.__doc__)
