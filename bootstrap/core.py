#!/usr/bin/env python3
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

# import logging
import os
import sys
import cmdline as cmd
from lexer import Lexer
from parser import Parser
from handler import Handler, Bundle

_tail_map = {
    'comp': '.ll',
    'hdr': '.defs',
    'diag': '.diag'}


def execute(args):
    """Perform verification and actions from cmdline"""

    # Validate input file
    srcsplit = args.source.split('.')
    if len(srcsplit) < 2 or srcsplit[1] != 'foidl':
        print("Input error: Source file doesn't have .foidl extension")
        return

    # Construct output file
    outhandler = None
    if not args.output:
        args.output = 'stdout'
        outhandler = sys.stdout
    else:
        outsplit = args.output.split('.')
        if len(outsplit) == 1:
            args.output = outsplit[0] + _tail_map[args.action]
        else:
            pass
        outhandler = open(args.output, "wt")

    # Read source in for all actions
    src = ""
    try:
        with open(args.source, 'rt') as fsrc:
            src = fsrc.read()
    except OSError as err:
        print("OS error: {}".format(err))
        return

    lexgen = Lexer()
    tokens = lexgen.get_lexer().lex(src)

    pargen = Parser(lexgen)
    pargen.parse()
    parser = pargen.get_parser()

    handler = Handler.handler_for(
        args.action,
        Bundle(
            args.source,
            parser.parse(tokens),
            outhandler, args.output))
    handler.validate()
    handler.emit()


def main():
    """Main entry point for parser bootstrap"""
    prog_name = os.path.basename(sys.argv[0])
    args = sys.argv[1:]
    if not args:
        args.append('-h')
    cparser = cmd.create_cmd_parser(prog_name)
    execute(cparser.parse_args(args))


main()
