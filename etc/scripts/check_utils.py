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

"""Module with utility functions for code checking scripts."""

import os
from ntpath import basename
from os import path
from subprocess import check_output, STDOUT, CalledProcessError, call
from pathlib import PurePath
from difflib import unified_diff


def read_file_lines(filepath):
    """Return a list of lines read from file."""
    with open(filepath, 'r', encoding='utf-8', newline=os.linesep) as f:
        return f.readlines()


def write_to_file(filepath, lines):
    """Write lines to file."""
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(''.join([line.replace('\r\n', '\n') for line in lines]))


def has_extension(filepath, extensions):
    """Check if file extension is in given extensions."""
    return any(filepath.endswith(ext) for ext in extensions)


def is_ignored(file, ignored):
    """Check if file is in the list of ignored files."""
    return any(i in PurePath(path.abspath(file)).parts for i in ignored)


def get_files_to_process(root_dir, ignored, extensions):
    """Get a list of files under root_dir with given extensions, omit ignored
    path parts."""
    if isinstance(ignored, str):
        ignored = [ignored]
    to_format = []
    for root, _, files in os.walk(root_dir):
        to_format.extend(path.join(root, file) for file in files
                         if has_extension(file, extensions) and not
                         is_ignored(path.join(root, file), ignored))
    return to_format


def get_diff_files_to_process(root_dir, ignored, extensions):
    """Get a list of changed files under root_dir with given extensions,
    omit ignored path parts."""
    to_format = []
    current_branch = check_output('git symbolic-ref --short HEAD', shell=True,
                                  cwd=root_dir).decode("UTF-8").strip()
    output = check_output((' ').join(('git diff --name-only', current_branch,
                                      'master')), shell=True,
                          cwd=root_dir).decode("UTF-8")
    for line in output.splitlines():
        file_name = basename(line)
        if line and has_extension(file_name, extensions) and \
                not is_ignored(line, ignored):
            to_format.append(path.join(root_dir, line))
    return to_format


def run(cmd, shell=False, cwd=None):
    """Execute subprocess, return exit code and output."""
    try:
        out = check_output(cmd, shell=shell, cwd=cwd, stderr=STDOUT)
    except CalledProcessError as ex:
        return ex.returncode, ex.output
    else:
        return 0, out


def check_diff(filepath, original, formatted):
    """Check diff between original and formatted lines, print diff and return
    True if diff occurs, return False otherwise."""
    diff = list(unified_diff(original, formatted))
    if diff:
        print('{} diff:'.format(filepath))
        print(("".join(diff)).replace('\r', ''))
        print()

    return bool(diff)
