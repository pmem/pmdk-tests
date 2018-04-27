#! /usr/bin/env python3
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
Remote 208-240V Digital Loggers Inc. Power Switch 1.8.1 handler.

Script for executing remote actions (OFF/ON/CYCLE) on selected machine
connected to Power Switch. Machine is identified either by its name or its
outlet number.
Script uses requests and lxml modules. To install requirements use
attached requirements.txt file:

$ pip3 install -r requirements.txt
"""


import argparse
import re
import sys
from time import sleep

from lxml import html
import requests

HOST = ''
LOGIN = ''
PASSWORD = ''
MACHINE = ''
OUTLET = ''
ACTION = ''
TIMEOUT = 10


def parse_cmd_args():
    global HOST, LOGIN, PASSWORD, MACHINE, OUTLET, ACTION, TIMEOUT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ip', '-i', required=True,
                        help='Power Switch IP address')
    parser.add_argument('--login', '-l', required=True,
                        help='Power Switch login')
    parser.add_argument('--password', '-p', required=True,
                        help='Power Switch password')
    parser.add_argument('--action', '-a', required=True,
                        help='Action executed on selected machine',
                        choices=['on', 'off', 'cycle'])
    parser.add_argument(
        '--timeout', '-t', type=int, default=10,
        help='Timeout in seconds between power off and on while running cycle')
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--machine', '-m', help='Machine name')
    group.add_argument('--outlet', '-o', help='Outlet number')

    args = parser.parse_args()

    HOST = args.ip
    LOGIN = args.login
    PASSWORD = args.password
    MACHINE = args.machine
    OUTLET = args.outlet
    ACTION = args.action
    TIMEOUT = args.timeout


def main():
    outlet_no = identify_outlet()
    make_action(outlet_no)


def identify_outlet():
    if OUTLET:
        return OUTLET
    return get_outlet_no()


def make_action(outlet_no):
    if ACTION == 'cycle':
        run_cycle(outlet_no)
    else:
        make_outlet_action(outlet_no, ACTION.upper().strip())


def get_outlet_no():
    r = requests.get('http://{}/index.htm'.format(HOST),
                     auth=(LOGIN, PASSWORD))
    r.raise_for_status()
    tree = html.fromstring(r.text)

    outlet_pattern = r'outlet\?(\d+)='
    table_cells = tree.xpath('//tr//td')
    if not table_cells:
        sys.exit('Error: HTML content of switch interface could not be parsed')

    found_cells = [c for c in table_cells if any(
        MACHINE in ctext for ctext in c.xpath('./text()'))]

    if not found_cells:
        sys.exit('Error: machine "{}" was not found in switch'.format(MACHINE))

    sibling_hrefs = found_cells[0].xpath('./following-sibling::*//a/@href')

    if not sibling_hrefs:
        sys.exit('Error: outlet for machine "{}" was not found',
                 format(MACHINE))

    outlet_match = re.match(outlet_pattern, sibling_hrefs[0])

    if not outlet_match:
        sys.exit('Error: outlet for machine "{}" was not found',
                 format(MACHINE))

    return outlet_match.group(1)


def run_cycle(outlet_no):
    make_outlet_action(outlet_no, 'OFF')
    sleep(TIMEOUT)
    make_outlet_action(outlet_no, 'ON')


def make_outlet_action(outlet_no, action):
    r = requests.get('http://{}/outlet?{}={}'.format(HOST, outlet_no, action),
                     auth=(LOGIN, PASSWORD))
    r.raise_for_status()


if __name__ == "__main__":
    parse_cmd_args()
    main()
