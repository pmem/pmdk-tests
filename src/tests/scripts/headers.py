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
This is a script that checks proper compilation of PMDK public headers.

Test checks:
-correcntess of compilation with all PMDK public headers for sample programs
 where each of available headers is in the first position at least once

Required arguments:
-c <compiler>   the compiler or compiler lists to be executed.
 Currently gcc, g++, clang, clang++ supported.
-i <includes>   path to PMDK public header files location.
"""

import argparse
import os
import subprocess

PROGRAM_PREFIX_NAME = 'header_test_'
PROGRAM_EXTENSION = '.cc'
WORK_PATH = './'
TEST_MSG = 'test PASSED!'

def pmdk_public_headers(include_path):
    """
    Returns list of header files acquired from given path
    """ 
    headers = []
    for root, dirs, files in os.walk(include_path):
        for file in files:
            if file.endswith(".h"):
                headers.append(os.path.join(root, file))
    return headers

def create_sample_programs(headers):
    """
    Creates files with sample c/c++ programs with PMDK includes where each of the headers
    is at least once at the top of 'includes' list
    """
    includes = []
    for header in headers:
        includes.append("#include \"{}\"".format(header))
    for index in range(len(includes)):
        with open(PROGRAM_PREFIX_NAME + str(index) + PROGRAM_EXTENSION, "w+") as f:
            f.write("\n".join(includes) + "\n\nint main() { return 0; }")
        if index != len(includes)-1:
            includes[0], includes[index + 1] = includes[index + 1], includes[0]

def compile_sample_programs(compilers, include_path):
    """
    Compiles sample programs with 'Wextra' and 'Werror' flag and cleans files and output files
    """
    for file in os.listdir(WORK_PATH):
        if PROGRAM_PREFIX_NAME in file and PROGRAM_EXTENSION in file:
            for compiler in compilers:
                if "/" in compiler:
                    output_file = file.rsplit('.', 1)[0] + (compiler).rsplit('/', 1)[1]
                else:
                    output_file = file.rsplit('.', 1)[0] + compiler
                compile_cmd = compiler + ' -I' + include_path + ' -Wextra -Werror ' + file + ' -o ' + output_file
                result = subprocess.run(compile_cmd, shell=True, stderr=subprocess.PIPE)
                if result.returncode != 0:
                    global TEST_MSG
                    TEST_MSG = 'test FAILED!'
                    print("\n\nCommand:\n" + result.args + " has failed!\nError msg:\n%s", result.stderr)
                # remove output file 
                subprocess.run("rm -f " + output_file, shell=True)
            # remove source file
            subprocess.run("rm -f " + file, shell=True)

def main():
    headers = pmdk_public_headers(args.includes)
    print("Creating sample programs...")
    create_sample_programs(headers)
    print("Compiling...")
    compile_sample_programs(args.compiler, args.includes)
    print(TEST_MSG)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Test verifies proper compilation with public pmdk headers.')
    parser.add_argument('-c', '--compiler',
                        #type=argparse.FileType('r', encoding='UTF-8'),
                        nargs='+', required=True,
                        help='List of compilers for compilation process')
    parser.add_argument('-i', '--includes',
                        required=True,
                        help='Path to pmdk includes.')
    args = parser.parse_args()
    main()
