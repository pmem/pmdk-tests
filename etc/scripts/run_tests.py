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


import xml.etree.ElementTree as ET
import sys
from subprocess import run, PIPE, TimeoutExpired
from argparse import ArgumentParser
from os import linesep, path
from shutil import rmtree
from pathlib import Path


def get_testdir_from_xml(binary_path):
    '''Acquire test directory from provided config.xml file.'''
    config_path = path.join(path.dirname(binary_path), 'config.xml')
    root = ET.parse(config_path).getroot()
    testdir_xpath = 'localConfiguration/testDir'
    elem = root.find(testdir_xpath)
    if elem is None:
        sys.exit(f'config.xml file invalid.'
                 f' Element {root.tag}/{testdir_xpath} not found.')
    workdir = elem.text
    return path.join(workdir, 'pmdk_tests')


def gtest_filter_rest(last_ran_test, all_tests, excluded):
    '''Prepare gtest_filter argument that filters out tests already executed \
    as well as excluded by user.'''
    gtest_filter = '--gtest_filter=-'
    already_ran = all_tests[:all_tests.index(last_ran_test) + 1]

    for test in already_ran:
        gtest_filter = gtest_filter + f'*.{test}:'
    if excluded:
        gtest_filter = gtest_filter + excluded
    return gtest_filter


def get_last_ran_test(output):
    '''Get last executed test from test binary execution output.'''
    for line in reversed(output.splitlines()):
        if '[ RUN      ]' in line:
            return line.split('.')[1].strip()


def get_fails(output):
    '''Get failed tests from test binary execution output.'''
    return [line.replace('[  FAILED  ]', '').split('(')[0].strip()
            for line in output.splitlines()
            if '[  FAILED  ]' in line and line.strip().endswith(')')]


def execute(cmd, timeout, testdir):
    '''Execute command, handle timeout, return output and exit code.'''
    try:
        proc = run(cmd, stdout=PIPE, timeout=timeout)
    except TimeoutExpired as e:
        print(e.output.decode('utf-8'))
        rmtree(testdir, ignore_errors=True)
        sys.exit('Execution timed out.')
    else:
        out = proc.stdout.decode('utf-8')
        returncode = proc.returncode
        print(out)
        return out, returncode


def get_all_tests_to_run(cmd):
    '''Call --gtest_list_tests on test binary and get all tests to be run.'''
    list_tests_out = run(cmd + ['--gtest_list_tests'],
                         stdout=PIPE).stdout.decode('utf-8')
    all_tests = [line.split('#')[0].strip()
                 for line in list_tests_out.splitlines()
                 if '#' in line or not line.endswith('.')]

    if not all_tests:
        sys.exit(f'No tests to run from {" ".join(cmd)}.')

    return all_tests


def last_test_terminated(out, returncode):
    """Last executed test terminated if in unsuccessful execution the last \
    '[ RUN     ]' doesn't have corresponding '[  FAILED  ]' afterwards.
    """
    no_fail_info = '[ RUN      ]' in next(line for line in reversed(
        out.splitlines()) if '[  FAILED  ]' in line or '[ RUN      ]' in line)
    return returncode != 0 and no_fail_info


def print_summary(failed, terminated, all_tests, binary):
    '''Print final execution summary.'''
    if failed:
        print(f'Out of {len(all_tests)} tests ran from {path.basename(binary)}'
              f' binary {len(failed)} failed:')
        for test in failed:
            print(test)

        print(f'{linesep}{len(terminated)} test(s) led to binary termination:')
        for test in terminated:
            print(test)


def execute_all_tests(binary, testdir, excluded, timeout):
    '''Run all tests from binary, check last ran test after finished process.
    Resume execution omitting already ran tests until all tests are run \
    or timeout occurs.
    '''
    cmd = [binary, f'--gtest_filter=-{excluded}'] if excluded else [binary]
    all_tests = get_all_tests_to_run(cmd)

    failing_tests = []
    terminating_tests = []

    out, returncode = execute(cmd, timeout, testdir)
    failing_tests.extend(get_fails(out))
    last_ran_test = get_last_ran_test(out)

    while last_ran_test != all_tests[-1]:
        print(f'{linesep}Test {last_ran_test} triggered execution'
              f' termination. Resuming execution.')
        terminating_tests.append(last_ran_test)
        rmtree(testdir, ignore_errors=True)

        cmd = [binary, gtest_filter_rest(last_ran_test, all_tests, excluded)]
        out, returncode = execute(cmd, timeout, testdir)
        last_ran_test = get_last_ran_test(out)
        failing_tests.extend(get_fails(out))

    if last_test_terminated(out, returncode):
        terminating_tests.append(last_ran_test)

    if terminating_tests:
        print_summary(failing_tests, terminating_tests, all_tests, binary)

    if terminating_tests or failing_tests:
        return 1

    return 0


if __name__ == '__main__':
    parser = ArgumentParser(
        description='Run tests from selected gtest binary.')
    parser.add_argument("-b", "--gtest-binary", required=True,
                        help="Path to gtest binary to be run.")
    parser.add_argument(
        '--timeout', help='Timeout in minutes, default: 60 minutes.'
                          ' Set to 0 for no timeout.', type=int, default=60)
    parser.add_argument(
        '-e', '--exclude', help='Tests to be excluded from'
                                ' execution (using gtest_filter semantics)')

    args = parser.parse_args()

    timeout = args.timeout * 60 if args.timeout else None  # minutes to seconds

    # check binary path first for more informative error message
    Path(args.gtest_binary).resolve(strict=True)

    testdir = get_testdir_from_xml(args.gtest_binary)

    exit_code = execute_all_tests(
        args.gtest_binary, testdir, args.exclude, timeout)

    sys.exit(exit_code)
