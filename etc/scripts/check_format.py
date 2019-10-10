#!/usr/bin/env python3
#
# Copyright 2018-2019, Intel Corporation
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

"""Check code style and formatting for selected files.

Check involves applying clang-format and checking format of test designs
documentation comments located above test fixture macros.

Requires clang-format in version 8 installed in the system.
"""

import sys
import argparse
import os
from os import path
import check_utils

DESIGN_START = '/**'
DESIGN_END = '*/'
DESIGN_EXTENSIONS = ('.cc',)
CPP_EXTENSIONS = ('.c', '.cc', '.cpp', '.h', '.tpp', '.hpp')
CLANG_FORMAT_BIN = ''
REQUIRED_VERSION = str(8)


def format_file(filepath):
    """Apply formatting rules to file, depending on its extension,
    return lists of lines before and after formatting."""
    original = check_utils.read_file_lines(filepath)
    formatted = original[:]

    if check_utils.has_extension(filepath, CPP_EXTENSIONS):
        formatted = clang_format(filepath)
    if check_utils.has_extension(filepath, DESIGN_EXTENSIONS):
        formatted = format_designs(formatted)

    return original, formatted


def format_designs(lines):
    """Format doxy-style test designs in given file lines."""
    indices = get_designs_line_indices(lines)
    if indices:
        return [format_design_line(
            line) if i in indices else line for i, line in enumerate(lines)]

    return lines


def get_designs_line_indices(lines):
    """Collect indices of lines with test designs."""
    iterations = range(len(lines))

    def end_found(i):
        """Check if the last line of test design section was found.
        'TEST_F' or 'TEST_P' occurs directly after the end of test design."""
        return lines[i].strip() == DESIGN_END and i + 1 in iterations \
            and ('TEST_F' in lines[i + 1] or 'TEST_P' in lines[i + 1])

    start_found = False
    all_designs_indices = []
    local_design_indices = []
    for i in iterations:
        if lines[i].strip() == DESIGN_START:
            local_design_indices = [i]
            start_found = True
            continue
        if start_found and end_found(i):
            all_designs_indices.extend(local_design_indices + [i])
            local_design_indices = []
            start_found = False
            continue
        if start_found and lines[i].strip().startswith('*'):
            local_design_indices.append(i)
        else:
            start_found = False

    return all_designs_indices


def format_design_line(line):
    """Apply formatting to test design line depending on its recognized type."""
    if line.strip() == DESIGN_START and not line.startswith(DESIGN_START):
        return line.strip() + os.linesep
    if line.strip() == DESIGN_END and not line.startswith(' ' + DESIGN_END):
        return ' ' + DESIGN_END + line.split(DESIGN_END)[1]
    if '*' in line and r'\li' in line and not line.startswith(
            r' *          \li'):
        return r' *          \li' + line.split(r'\li')[1]
    if r'\test' in line and not line.startswith(r' * \test'):
        return r' * \test' + line.split(r'\test')[1]
    if line.strip().startswith('*') and not line.startswith(' *'):
        return ' *' + line.split('*')[1]

    return line


def check_prerequisites():
    """Check if prerequisites for code formatting exist on system."""
    global CLANG_FORMAT_BIN

    CLANG_FORMAT_BIN = find_clang_format()
    if not CLANG_FORMAT_BIN:
        sys.exit('No clang-format in version ' + REQUIRED_VERSION + ' found.')


def find_clang_format():
    """Find clang-format binary with appropriate version on system."""

    returncode, out = check_utils.run('clang-format --version', shell=True)
    if returncode == 0 and 'version ' + REQUIRED_VERSION in str(out):
        return 'clang-format'

    alternative_binary = 'clang-format-' + REQUIRED_VERSION
    returncode, _ = check_utils.run(alternative_binary + ' --version', shell=True)
    if returncode == 0:
        return alternative_binary

    return ''


def clang_format(filepath):
    """Apply clang-format to file."""
    cmd = '{} "{}" -style=file'.format(CLANG_FORMAT_BIN, filepath)
    returncode, out = check_utils.run(cmd, shell=True)
    if returncode != 0:
        sys.exit('{}{}{} exited with code {}.'
                 .format(out.decode('utf-8'), os.linesep, cmd, returncode))
    return out.decode('utf-8').splitlines(keepends=True)


def get_cmd_args():
    """Parse and return command line arguments."""
    class CheckPrerequisitesAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            check_prerequisites()
            parser.exit()

    parser = argparse.ArgumentParser(
        description='Check format of code and test designs.')
    parser.add_argument('-p', '--path',
                        help='Path to the file to be formatted or directory'
                             ' if in recursive mode.', required=True)
    parser.add_argument(
        '-r', '--recursive', action='store_true', default=False,
        help='Recursive mode with root directory provided by'
             ' -p/--path argument')
    parser.add_argument('-i', '--in-place', action='store_true', default=False,
                        help='Apply edits to files and display diffs')
    parser.add_argument('--ignore', default=['build'], nargs='+',
                        help='In recursive mode ignore elements with given '
                             'name in path. Default: build')
    parser.add_argument('--check-prerequisites', nargs=0,
                        help='Check prerequisites only.',
                        action=CheckPrerequisitesAction)
    parser.add_argument('--all', action='store_true', default=False,
                        help='Check all files in repository')
    return parser.parse_args()


def main():
    """Check and/or apply formatting in-place to all selected and appropriate
    files according to command line arguments. Script ends with failure when
    checking files without formatting them ("-i" param not used) and
    issues (diffs) found.
    """
    args = get_cmd_args()
    check_prerequisites()
    files_to_process = []

    if args.all:
        if args.recursive:
            if not path.isdir(args.path):
                sys.exit('{} is not a directory'.format(
                    path.abspath(args.path)))
            files_to_process = check_utils.get_files_to_process(
                args.path, args.ignore, DESIGN_EXTENSIONS + CPP_EXTENSIONS)
        else:
            if not path.isfile(args.path):
                sys.exit('{} is not a regular file'.format(
                    path.abspath(args.path)))
            files_to_process.append(args.path)
    else:
        files_to_process = check_utils.get_diff_files_to_process(
            args.path, args.ignore, DESIGN_EXTENSIONS + CPP_EXTENSIONS)

    diffs_after_formatting = False
    for filepath in files_to_process:
        original, formatted = format_file(filepath)
        diff_occurs = check_utils.check_diff(filepath, original, formatted)
        if diff_occurs:
            diffs_after_formatting = True
            if args.in_place:
                check_utils.write_to_file(filepath, formatted)

    if not args.in_place and diffs_after_formatting:
        sys.exit(1)


if __name__ == '__main__':
    main()
