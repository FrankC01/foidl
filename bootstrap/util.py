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
import logging
import pprint

from enums import ParseLevel
from lexer import Lexer as full_lex
from hlexer import HLexer as lite_lex
from fparser import AParser as full_parse
from hparser import HParser as lite_parse

LOGGER = logging.getLogger()


def file_exists(path, base, ext):
    ffile = os.path.join(path, base) + '.' + ext
    LOGGER.debug("         checking for {}".format(ffile))
    return ffile if os.path.isfile(ffile) else None


def validate_file(infile, ext):
    """Validate a file existence and type"""
    absfile = os.path.abspath(infile)
    if not os.path.isfile(absfile):
        raise IOError("{} is not a file".format(absfile))

    head, tail = os.path.split(absfile)
    srcsplit = tail.split('.')

    # Validate input file
    if len(srcsplit) < 2 or srcsplit[1] != ext:
        raise IOError(
            "{} doesn not have {} extension".format(absfile, ext))
    return head + "/", absfile


def absolutes_path_for(ins):
    if type(ins) is list:
        return [os.path.abspath(x) for x in ins]
    else:
        return os.path.abspath(ins)


def parse_file(srcfile, state, level=ParseLevel.FULL):
    """Read and parse some kind of source file

    With relevent lexer and parser context
    """
    if level is ParseLevel.FULL:
        lexg = full_lex
        prsg = full_parse
    else:
        lexg = lite_lex
        prsg = lite_parse

    src = ""
    # Read in source file
    try:
        with open(srcfile, 'rt') as fsrc:
            src = fsrc.read()
    except OSError as err:
        raise("OS error: {}".format(err))

    # Lex source
    lexgen = lexg()
    tokens = lexgen.get_lexer().lex(src)

    # Parse tokens
    pargen = prsg(lexgen, srcfile)
    pargen.parse()
    parser = pargen.get_parser()
    # if level is ParseLevel.FULL:
    #     pprint.pprint(parser.lr_table.grammar.__dict__)
    return parser.parse(tokens, state=state).module()
