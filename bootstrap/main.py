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
import warnings
import logging
from colorlog import ColoredFormatter

import util
import errors
import cmdline as cmd
from enums import literal_dict, ParseLevel
from handler import Handler, Bundle, State, preprocess_runtime
from symbol import SymbolTree

LOGGER = None


def create_console_handler(logger, verbose_level):
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

    if verbose_level == 1:
        logger.setLevel(logging.INFO)
    elif verbose_level == 2:
        logger.setLevel(logging.DEBUG)
    elif verbose_level == 3:
        logger.setLevel(logging.WARN)
    else:
        pass
    return logger


def setup_loggers(verbose_level):
    logger = logging.getLogger()
    return create_console_handler(logger, verbose_level)


_tail_map = {
    'comp': '.ll',
    'compr': '.ll',
    'hdr': '.defs',
    'diag': '.diag'}


def execute(args):
    """Perform verification and actions from cmdline"""
    try:
        absfile = util.validate_file(args.source, 'foidl')
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
        lvl = ParseLevel.LITE if args.action == 'hdr' else ParseLevel.FULL
        hdronly = True if args.action == 'hdr' else False
        state = preprocess_runtime(
            State(
                literal_dict(),
                SymbolTree(absfile),
                util.absolutes_path_for(args.inc_paths),
                hdronly),
            args)

        state.mainsrc = absfile
        handler = Handler.handler_for(
            args.action,
            Bundle(
                util.absolutes_path_for(args.inc_paths),
                absfile,
                util.parse_file(absfile, state, lvl),
                state,
                outhandler, args.output, args.rt))

        handler.validate()
        handler.emit()
    except (errors.PFoidlError, IOError) as err:
        LOGGER.error("{}: {}".format(type(err).__name__, err))
        return


def main(prog_name=sys.argv[0], args=sys.argv[1:]):
    """Main entry point for parser bootstrap"""
    global LOGGER
    if not args:
        args.append('-h')
    else:
        pass

    cparser = cmd.create_cmd_parser(prog_name)
    argres = cparser.parse_args(args)

    LOGGER = setup_loggers(argres.llevel)

    execute(argres)


if __name__ == "__main__":
    main()
