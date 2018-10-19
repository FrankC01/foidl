# ------------------------------------------------------------------------------
# Copyright 2018 Frank V. Castellucci
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------

import os
import argparse

"""Command line parser menu"""


def create_cmd_parser(prog_name):
    """Bootstrap command line parser
    """
    aparser = argparse.ArgumentParser(
        prog=prog_name,
        description='Bootstrap FOIDL Compiler generator.',
        epilog="""
Action (-a) set (one of):
    comp  - compiles the input to LLVM-IR (default)
    compr - runtime library compile to LLVM-IR
    hdr   - generate a header file for input
    diag  - generate parser diagnostics

Message Verbosity (-v):
    none    - Displays only critical and error messages (default)
    -v      - Informational messages
    -vv     - Include debug messages
    -vvv    - Warning messages only
        """,
        formatter_class=argparse.RawTextHelpFormatter)

    aparser.add_argument(
        '-v',
        action='count',
        default=0,
        dest='llevel',
        help='Increase output sent to stderr (see Verbosity below)')

    aparser.add_argument(
        '-a',
        help='action to take (see Action below)',
        default='comp',
        choices=['comp', 'compr', 'hdr', 'diag'],
        dest='action',
        metavar='action set')

    aparser.add_argument(
        '-s',
        help='foidl source',
        required=True,
        dest='source',
        metavar='<file>')

    aparser.add_argument(
        '-r',
        help='ignore runtime headers',
        dest='rt',
        default=False,
        action='store_true')

    aparser.add_argument(
        '-o',
        help='output file - default to stdout if not set',
        dest='output',
        metavar='<file>')

    aparser.add_argument(
        '-t',
        help='target platform triple - default to host',
        dest='triple',
        metavar='triple')

    aparser.add_argument(
        '-I',
        help='Path(s) for include (.defs) files',
        dest='inc_paths',
        nargs='*',
        default=[],
        metavar='<p>')

    return aparser
