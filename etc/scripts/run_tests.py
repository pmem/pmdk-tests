#!/usr/bin/env python3
#
# Copyright 2017, Intel Corporation
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


from subprocess import run, PIPE, TimeoutExpired
from argparse import ArgumentParser
from os import listdir, unlink, linesep
from os.path import isfile, isdir, join, basename, dirname
from shutil import rmtree
from sys import exit
import xml.etree.ElementTree as ET


def get_workdir_from_xml(binary_path):
    config_path = join(dirname(binary_path), 'config.xml')
    root = ET.parse(config_path).getroot()
    workdir_xpath = 'localConfiguration/testDir'
    elem = root.find(workdir_xpath)
    if elem is None:
        exit(f'config.xml file invalid. Element {root.tag}/{workdir_xpath} not found.')
    workdir = elem.text
    return join(workdir, 'pmdk_tests')

def gtest_filter_rest(last_ran_test, all_tests, excluded):
    gtest_filter = '--gtest_filter=-'
    already_ran = all_tests[:all_tests.index(last_ran_test) + 1]

    for test in already_ran:
        gtest_filter = gtest_filter + f'*.{test}:'
    if excluded:
        gtest_filter = gtest_filter + excluded
    return gtest_filter

def get_last_ran_test(output):
    for line in reversed(output.splitlines()):
        if '[ RUN      ]' in line:
            return line.split('.')[1].strip()


def get_fails(out):
    return [line.replace('[  FAILED  ]', '').split('(')[0].strip()
            for line in out.splitlines()
            if '[  FAILED  ]' in line and line.strip().endswith(')')]

def execute(cmd, timeout, workdir):
    proc = None
    try:
        proc = run(cmd, stdout=PIPE, timeout=timeout)
    except TimeoutExpired as e:
        print(e.output.decode('utf-8'))
        rmtree(workdir)
        exit('Execution timed out.')
    out = proc.stdout.decode('utf-8')
    returncode = proc.returncode
    print(out)
    return out, returncode

def get_all_tests_to_run(cmd):
    list_tests_out = run(cmd + ['--gtest_list_tests'],
                         stdout=PIPE).stdout.decode('utf-8')
    all_tests = [test.split('#')[0].strip()
                 for test in list_tests_out.splitlines() if not test.endswith('.')]

    if not all_tests:
        exit(f'No tests to run from {" ".join(cmd)}.')

    return all_tests

def last_test_terminated(out, returncode):
    # last test terminated if in non-succesful execution the last '[ RUN     ]'
    # doesn't have corresponding '[  FAILED  ]' afterwards
    no_fail_info = '[ RUN      ]' in next(line for line in reversed(
        out.splitlines()) if '[  FAILED  ]' in line or '[ RUN      ]' in line)
    return returncode != 0 and no_fail_info

def execute_whole_binary(binary, workdir, excluded, timeout):
    cmd = [binary, f'--gtest_filter=-{excluded}'] if excluded else [binary]
    all_tests = get_all_tests_to_run(cmd)

    failing_tests = []
    terminating_tests = []

    out, returncode = execute(cmd, timeout, workdir)
    failing_tests.extend(get_fails(out))
    last_ran_test = get_last_ran_test(out)

    while last_ran_test != all_tests[-1]:
        print(f'{linesep}Test {last_ran_test} triggered execution termination. Resuming execution.')
        terminating_tests.append(last_ran_test)
        rmtree(workdir)

        cmd = [binary, gtest_filter_rest(last_ran_test, all_tests, excluded)]
        out, returncode = execute(cmd, timeout, workdir)
        last_ran_test = get_last_ran_test(out)
        failing_tests.extend(get_fails(out))

    if last_test_terminated(out, returncode):
        terminating_tests.append(last_ran_test)

    if terminating_tests:
        print_summary(failing_tests, terminating_tests, all_tests, binary)

    if terminating_tests or failing_tests:
        return 1
    return 0

def print_summary(failed, terminated, all, binary):
    if failed:
        print(f'Out of {len(all)} tests ran from {basename(binary)} binary {len(failed)} failed:')
        for test in failed:
            print(test)

        print(f'{linesep}{len(terminated)} test(s) led to binary termination:')
        for test in terminated:
            print(test)

if __name__ == '__main__':
    parser = ArgumentParser(
        description='Run tests from selected gtest binary.')
    parser.add_argument("-b", "--gtest-binary", required=True,
                        help="Path to gtest binary to be run.")
    parser.add_argument(
        '--timeout', help='Timeout in minutes, default: 60 minutes. Set to 0 for no timeout.', type=int, default=60)
    parser.add_argument(
        '-e', '--exclude', help='Tests to be excluded from execution (using gtest_filter semantics)')
    args = parser.parse_args()

    timeout = args.timeout * 60 if args.timeout else None  # minutes to seconds
    workdir = get_workdir_from_xml(args.gtest_binary)

    exit_code = execute_whole_binary(
        args.gtest_binary, workdir, args.exclude, timeout)

    exit(exit_code)
