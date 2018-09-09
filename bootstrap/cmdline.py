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

import argparse

"""Command line parser menu"""


def create_cmd_parser(prog_name):
    """Bootstrap command line parser
    """
    aparser = argparse.ArgumentParser(
        prog=prog_name,
        description='Bootstrap FOIDL Compiler generator.',
        epilog="""Action set (one of):
        comp - compiles the input to LLVM-IR (default)
        hdr  - generate a header file for input
        diag - generate parser diagnostics
        """,
        formatter_class=argparse.RawTextHelpFormatter)

    aparser.add_argument(
        '-s',
        help='foidl source',
        required=True,
        dest='source',
        metavar='<file>')

    aparser.add_argument(
        '-o',
        help='output file - default to stdout',
        dest='output',
        metavar='<file>')

    aparser.add_argument(
        '-t',
        help='target platform triple - default to host',
        dest='triple',
        metavar='triple')

    aparser.add_argument(
        '-I',
        help='Path(s) to search for include (.defs) files',
        dest='inc_paths',
        nargs='*',
        default=[],
        metavar='<path>')

    aparser.add_argument(
        '-a',
        help='action to take (defaults to comp)',
        default='comp',
        choices=['comp', 'hdr', 'diag'],
        dest='action',
        metavar='action set')

    return aparser
