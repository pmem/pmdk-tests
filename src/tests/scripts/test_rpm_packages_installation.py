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
This module includes tests which check which rpm packages from PMDK library
are not installed correctly.

Tests check:
-compatibility of the version of installed rpm packages from PMDK library,
-if all rpm packages from PMDK library are installed.

Required arguments:
-r <PMDK_path>    the PMDK library root path.
"""

from os import listdir, path, linesep
from subprocess import check_output
import unittest
import sys
import re

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


def get_names_of_rpm_content():
    """
    Returns names of elements, for which are installed packages from PMDK
    library.
    """
    rpm_packages_path = path.join(pmdk_path, 'rpm', SYSTEM_ARCHITECTURE)
    installed_rpm_packages = check_output(
        'ls | grep ' + PMDK_VERSION, cwd=rpm_packages_path, shell=True)
    installed_rpm_packages = installed_rpm_packages.decode(
        'UTF-8').split(linesep)
    libraries_names = [item.split(
        '-')[0] for item in installed_rpm_packages if item.split('-')[0]]
    return set(libraries_names)


def get_uninstalled_rpm_packages():
    """
    Returns names of rpm packages from PMDK library, which are not installed.
    """
    libraries = get_names_of_rpm_content()
    uninstalled_packages = []
    for library in libraries:
        if library == "pmempool" and check_output(
                'find /usr/bin/ -name ' + library, cwd=pmdk_path, shell=True):
            pass
        elif library == "libpmemobj++" and check_output(
                'find /usr/include/' + library + ' -name *.hpp', cwd=pmdk_path,
                shell=True):
            pass
        elif library == "pmdk":
            pass
        elif check_output('find /usr/lib64/ -name ' + library + '.so',
                          cwd=pmdk_path, shell=True):
            pass
        else:
            uninstalled_packages.append(library)
    return uninstalled_packages


def get_incompatibility_packages():
    """
    Returns names of rpm packages from PMDK library, which are not compatible
    with the actual version of PMDK library.
    """
    pkgconfig_directory = '/usr/lib64/pkgconfig/'
    incompatibility_packages = []
    libraries = get_names_of_rpm_content()
    for library in libraries:
        if library in ['pmdk', 'pmempool']:
            continue
        version = (check_output('cat ' + library + '.pc',
                                cwd=pkgconfig_directory, shell=True).
                   decode('UTF-8').split(linesep)[0].split('=')[1])
        if not version in PMDK_VERSION.replace('~', '-'):
            incompatibility_packages.append(library)
    return incompatibility_packages


class TestBuildRpmPackages(unittest.TestCase):

    def test_compatibility_of_version_of_installed_rpm_packages(self):
        """
        Checks if the version of installed rpm packages is correct.
        """
        incompatibility_packages = get_incompatibility_packages()
        error_msg = linesep + 'List of incompatibility packages: '
        for package in incompatibility_packages:
            error_msg += linesep + package
        self.assertFalse(incompatibility_packages, error_msg)

    def test_correctness_of_installed_rpm_packages(self):
        """
        Checks if all rpm packages from PMDK library are installed.
        """
        uninstalled_packages = get_uninstalled_rpm_packages()
        error_msg = linesep + 'List of uninstalled packages: '
        for package in uninstalled_packages:
            error_msg += linesep + package
        self.assertFalse(uninstalled_packages, error_msg)


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
        pmdk_path = parse_argument('-r')
        PMDK_VERSION, SYSTEM_ARCHITECTURE =\
            get_package_version_and_system_architecture(pmdk_path)
        unittest.main()
    else:
        print(__doc__)
        print(unittest.main.__doc__)
