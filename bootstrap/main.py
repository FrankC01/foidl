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

import sys
import util
import warnings
import logging
from colorlog import ColoredFormatter

import cmdline as cmd
from lexer import Lexer
from parser import Parser
from handler import Handler, Bundle

LOGGER = None


def create_console_handler(logger, verbose_level=0):
    clog = logging.StreamHandler()
    clog.setFormatter(
        ColoredFormatter(
            "%(log_color)s[%(asctime)s %(levelname)-8s%(module)s]%(reset)s "
            "%(white)s%(message)s",
            datefmt="%H:%M:%S",
            reset=True,
            log_colors={
                'DEBUG': 'cyan',
                'INFO': 'green',
                'WARNING': 'yellow',
                'ERROR': 'red',
                'CRITICAL': 'red',
            }))

    logger.addHandler(clog)
    logger.propagate = False

    if verbose_level == 0:
        logger.setLevel(logging.INFO)
    elif verbose_level == 1:
        logger.setLevel(logging.WARN)
    else:
        logger.setLevel(logging.DEBUG)
    return logger


def setup_loggers(verbose_level):
    logger = logging.getLogger()
    return create_console_handler(logger, verbose_level)


_tail_map = {
    'comp': '.ll',
    'hdr': '.defs',
    'diag': '.diag'}


def execute(args):
    """Perform verification and actions from cmdline"""
    try:
        absfile = util.validate_file(args.source, 'foidl')
    except IOError as err:
        LOGGER.error("{}".format(err))
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
        outhandler = open(args.output, "wt+")

    # Supress python warnings
    if not sys.warnoptions:
        warnings.simplefilter("ignore")

    handler = Handler.handler_for(
        args.action,
        Bundle(
            util.absolutes_path_for(args.inc_paths),
            absfile,
            util.parse_file(absfile, Lexer, Parser),
            outhandler, args.output))

    handler.validate()
    handler.emit()


def main(prog_name=sys.argv[0], args=sys.argv[1:]):
    """Main entry point for parser bootstrap"""
    global LOGGER
    if not args:
        args.append('-h')
    else:
        pass

    LOGGER = setup_loggers(0)

    cparser = cmd.create_cmd_parser(prog_name)
    execute(cparser.parse_args(args))


main()
