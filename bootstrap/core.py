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
# import logging
import os
import sys
from pprint import pprint
from lexer import Lexer
from parser import Parser
from ast import ast_trace

# test_input = """
# module foo

# include [fizz bar bap]

# var myreal 10.5
# var myint 5
# var yourint 5
# var anint 15
# var mystr "Sammy Davis Jr."
# var mychar 'c'

# var myvect <>
# var mylist []
# var mymap {}
# var myset #{}

# var mappairs {:a 5 :b 6 :c}

# func t[x y z]
# """

test_input = """
module foo
;var a 7
;var a 5
func t[a b]
    foo: 1 2
    let [a add: 1 2] @()
;    fiz: 3 4
"""


def create_parser(prog_name):
    """Bootstrap command line parser
    """
    aparser = argparse.ArgumentParser(
        prog=prog_name,
        description='Bootstrap FOIDL generator.',
        epilog='This process is required to build FOIDL',
        formatter_class=argparse.RawDescriptionHelpFormatter)

    aparser.add_argument(
        '-t',
        help='target platform for bootstrap')
    aparser.add_argument(
        '-i',
        help='include path(s)')
    aparser.add_argument(
        '-a',
        help='AST evaluate - Debugging')
    aparser.add_argument(
        '-g',
        help='Dump Grammar - Debugging')
    return aparser


def main(prog_name=os.path.basename(sys.argv[0]), args=None,
         with_loggers=True):
    """Main entry point for parser bootstrap
    """
    if args is None:
        args = sys.argv[1:]
    aparser = create_parser(prog_name)
    args = aparser.parse_args(args)
    mlex = Lexer()
    lexer = mlex.get_lexer()
    tokens = lexer.lex(test_input)
    # for n in tokens:
    #     print(n)
    pg = Parser(mlex)
    pg.parse()
    parser = pg.get_parser()
    ast = parser.parse(tokens)
    print("AST Type {} => {}".format(type(ast), ast))
    # print("Args = {}".format(args.a))
    if args.a:
        if len(ast.literals):
            pprint(ast.literals)
        # ast.eval()
        ast_trace(ast)
    if args.g:
        pprint(parser.lr_table.grammar.__dict__)


if __name__ == '__main__':
    main()
