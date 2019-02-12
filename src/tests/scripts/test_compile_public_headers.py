#!usr/bin/env python3
#
# Copyright 2019, Intel Corporation
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
Checks proper compilation of PMDK public headers using sample programs
where each of available headers is at the first position at least once

Currently gcc, g++, clang and clang++ compilers are supported
"""

import argparse
import os
import subprocess
from pathlib import Path
import sys
import logging

PROGRAM_PREFIX_NAME = 'header_test_'
PROGRAM_EXTENSION = '.cc'
WORK_PATH = './'
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s"
    )


def pmdk_public_headers(include_path):
    """
    Returns list of header files acquired from given path
    """
    headers = []
    for root, _, files in os.walk(include_path):
        for file in files:
            if file.endswith(".h"):
                headers.append(os.path.join(root, file))
    if not headers:
        logging.error("Wrong or empty includes directory")
        sys.exit(1)
    return headers


def create_sample_programs(headers):
    """
    Creates files with sample c/c++ programs with PMDK includes where each of the headers
    is at least once at the top of 'includes' list
    """
    logging.info("Creating sample programs...")
    includes = []
    for header in headers:
        includes.append("#include \"{}\"".format(header))
    for index in range(len(includes)):
        with open(PROGRAM_PREFIX_NAME + str(index) + PROGRAM_EXTENSION, "w+") as file:
            file.write("\n".join(includes) + "\n\nint main() { return 0; }")
        if index != len(includes) - 1:
            includes[0], includes[index + 1] = includes[index + 1], includes[0]


def compile_sample_programs(compilers, include_path):
    """
    Compiles sample programs with 'Wextra', 'Werror' and 'Wpedantic' flag
    and cleans files and output files
    """
    files = [file for file in os.listdir(WORK_PATH)
             if PROGRAM_PREFIX_NAME in file and PROGRAM_EXTENSION in file]
    for index, file in enumerate(files):
        for compiler in compilers:
            output_file = file.rsplit('.', 1)[0] + Path(compiler).name
            compile_cmd = "{} -I {} -Wextra -Werror -Wpedantic {} -o {}".format(
                compiler, include_path, file, output_file)
            try:
                subprocess.check_output(compile_cmd, stderr=subprocess.STDOUT,
                                        universal_newlines=True, shell=True)
            except subprocess.CalledProcessError as err:
                ret_code, cmd = err.args
                logging.error("Compiling with %s... FAILED\nCommand:\n%s\nOutput:\n%s",
                              compiler, cmd, err.output)
                for program_file in os.listdir(WORK_PATH):
                    if PROGRAM_PREFIX_NAME in program_file:
                        os.remove(program_file)
                return ret_code
            if index == len(files) - 1:
                logging.info("Compiling with %s... OK", compiler)
            # remove output file
            os.remove(output_file)
        # remove source file
        os.remove(file)
    logging.info("test PASSED!")
    return 0


def main():
    headers = pmdk_public_headers(args.includes)
    create_sample_programs(headers)
    sys.exit(compile_sample_programs(args.compiler, args.includes))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Test verifies proper compilation' +
                                                 'with public pmdk headers.')
    parser.add_argument('-c', '--compiler',
                        nargs='+', required=True,
                        help='List of supported compilers')
    parser.add_argument('-i', '--includes',
                        required=True,
                        help='Path to pmdk includes.')
    args = parser.parse_args()
    main()
