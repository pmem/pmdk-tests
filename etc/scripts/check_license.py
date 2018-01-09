#!/usr/bin/env python3
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

"""Check license header compliance with LICENSE file and last modification date
for all files in the project that require license.
"""

import sys
import re
import argparse
from os import path
import check_utils

COPYRIGHT_HEADER_PATTERN = r'^(.*)Copyright (.*), Intel Corporation'
LICENSE_EXTENSIONS = ('.cc', '.cpp', '.h', '.sh',
                      '.py', '.tpp', '.hpp', '.txt', '.cmake')
IGNORED_FILES = ['__init__.py']
ERRORS = []


def validate_license_file(license_filepath):
    """Checks that LICENSE file contains copyright header on its first line."""
    lines = check_utils.read_file_lines(license_filepath)
    if not re.search(COPYRIGHT_HEADER_PATTERN, lines[0]):
        sys.exit('Valid copyright header not found in the first line of {}'
                 .format(license_filepath))


def check_license_date(filepath, copyright_line):
    """Check if license date in file matches file's last modification date."""
    cmd = 'git log -1 --format="%ad" --date=format:"%Y" "{}"'.format(filepath)
    cwd = path.dirname(path.abspath(filepath))
    returncode, out = check_utils.run(cmd, cwd=cwd, shell=True)

    if returncode != 0:
        print(out.decode('utf-8'))
        sys.exit('{} exited with {}.'.
                 format(cmd, returncode))

    modification_year = out.decode('utf-8').strip()
    if modification_year not in copyright_line:
        msg = '{}: last modification year ({}) not found in "{}"'
        ERRORS.append(msg.format(filepath, modification_year,
                                 copyright_line.rstrip()))


def get_first_line_with_license(file_lines):
    """Find line in file containing copyright header, return its index or
    return -1 if line was not found."""
    license_start = -1
    for i, line in enumerate(file_lines):
        if re.search(COPYRIGHT_HEADER_PATTERN, line):
            license_start = i
            break

    return license_start


def check_license(filepath, license_lines):
    """Checks if license text matches LICENSE file and copyright date matches
    last modification date."""
    lines = check_utils.read_file_lines(filepath)
    license_start = get_first_line_with_license(lines)

    if license_start == -1:
        ERRORS.append('{}: license could not be found'.format(filepath))
        return

    check_license_date(filepath, lines[license_start])

    # index + 1 to omit year-dependent copyright header
    for f_line, l_line in zip(lines[license_start + 1:], license_lines[1:]):
        if l_line.strip() not in f_line:
            ERRORS.append('{}: license text mismatch with line "{}"'
                          .format(filepath, l_line.strip()))


def main():
    """Parse command line arguments, get and check all files under root
    directory that require license, print errors.
    """
    parser = argparse.ArgumentParser(
        description='Check license in repository.')

    parser.add_argument('-p', '--path',
                        help='Path to repository root dir', required=True)
    parser.add_argument('--ignore',
                        help='Ignore elements with given name in path element.'
                             ' Default: build', default=['build'], nargs='+')
    args = parser.parse_args()
    license_filepath = path.join(args.path, 'LICENSE')
    validate_license_file(license_filepath)
    root_dir = path.abspath(args.path)

    if not path.isdir(root_dir):
        sys.exit('{} is not a directory'.format(path.abspath(args.path)))
    files_to_process = check_utils.get_files_to_process(
        root_dir, args.ignore + IGNORED_FILES, LICENSE_EXTENSIONS)

    for filepath in files_to_process:
        license_lines = check_utils.read_file_lines(license_filepath)
        check_license(filepath, license_lines)

    if ERRORS:
        for line in ERRORS:
            print(line)
        sys.exit(1)


if __name__ == '__main__':
    main()
