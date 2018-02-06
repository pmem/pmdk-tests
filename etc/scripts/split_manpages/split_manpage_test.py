#!usr/bin/env python3

from subprocess import STDOUT, PIPE, run
from os import listdir, path
import unittest
import sys
from argparse import ArgumentParser
import split_manpage as split_man


class TestDocumentation(unittest.TestCase):

    def test_1(self):
        """
        Test check if all extracted macros from header files are in "Generated" catalog
        """
        missing_mac = split_man._check_completness_macros_in_gen(PMDK_PATH)
        msg = '\nList of missing macros in \"Generetaed\" catalog: '
        for mac in missing_mac:
            msg = msg + '\n' + mac
        self.assertEqual(len(missing_mac), 0, msg)

    def test_2(self):
        """
        Test check if all extracted functions from .so files are in "Generated" catalog
        """
        missing_funcs = split_man._check_completness_funcs_in_gen(PMDK_PATH)
        msg = '\nList of missing functions in \"Generetaed\" catalog: '
        for func in missing_funcs:
            msg = msg + '\n' + func
        self.assertEqual(len(missing_funcs), 0, msg)

    def test_3(self):
        """
        Test check if all functions and macros from "Generated" catalog can be extracted 
        from headers files and .so files
        """
        missing_funcs_and_mac = split_man._check_completness_extract_funcs_and_macros(
            PMDK_PATH)
        msg = '\nList of missing macros and functions: '
        for elem in missing_funcs_and_mac:
            msg = msg + '\n' + elem
        self.assertEqual(len(missing_funcs_and_mac), 0, msg)

    def test_4(self):
        """
        Test check if all functions and macros are splitted with man page
        """
        no_link_funcs_and_mac = split_man._check_linking_manpages(PMDK_PATH)
        msg = '\nList of macros and functions without linked manpage: '
        for elem in no_link_funcs_and_mac:
            msg = msg + '\n' + elem
        self.assertEqual(len(no_link_funcs_and_mac), 0, msg)


if __name__ == '__main__':

    PMDK_PATH = sys.argv[len(sys.argv) - 1]
    sys.argv.pop()
    unittest.main()
