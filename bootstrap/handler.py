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
import functools

import ast
from ptree import ParseTree, ParseSymbol
import util
import pprint
from emit import LlvmGen
from enums import ExpressionType
from errors import NotFoundError

from abc import ABC, abstractmethod

LOGGER = logging.getLogger()

_std_comment = """
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Generated by pfoidl - Bootstrap FOIDL Compiler generator
; Copyright (c) Frank V. Castellucci
; All rights reserved
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
"""


def _build_comp(*functions):
    """Construct composition"""
    return functools.reduce(
        lambda f, g: lambda x: f(g(x)), functions, lambda x: x)


def _parse(input):
    """Compiles file"""
    srcfile, state = input
    LOGGER.debug("Parsing {}".format(srcfile))
    fpart = os.path.split(srcfile)[1].split('.')[0]
    state.symtree.push_scope(fpart, srcfile)
    ast = util.parse_file(srcfile, state)
    # state.symtree.push_scope(ast.name, srcfile)
    return state, ast


def _top_symbols(input):
    """Extracts var and func symbol refs only"""
    state, mod = input
    for t in mod.value:
        if type(t) is ast.Variable:
            state.symtree.register_symbol(t.name, t.get_reference())
        elif type(t) is ast.Function:
            state.symtree.register_symbol(t.name, t.get_reference())
    return input


def _inc_to_syms():
    return _build_comp(_top_symbols, _parse)


_HDR_PARSE = _parse  # _inc_to_syms()


def _resolve_header(fname, ipaths):
    """_resolve_header resolves absolute file location"""
    for p in ipaths:
        hfile = util.file_exists(p, fname, 'defs')
        if hfile:
            return hfile
    return None


def preprocess_runtime(state, args):
    """Pre process well known runtime includes"""

    # Extend -I paths
    if "FOIDLC2_PATH" in os.environ:
        state.inclpath.insert(
            0,
            os.environ.get("FOIDLC2_PATH"))

    # If not skipping run-time (used to compiler run-time)
    # preprocess well known headers
    if not args.rt:
        includes = ['foidlrt', 'langcore']
        inc_files = []
        for i in includes:
            ffile = _resolve_header(i, state.inclpath)
            if ffile:
                inc_files.append(ffile)
            else:
                raise NotFoundError(
                    "Unable to resolve include file {}".format(i))
        [_HDR_PARSE((x, state)) for x in inc_files]

    return state


def include_parse(state, obj):
    inc_files = []
    for i in obj.value:
        ffile = _resolve_header(i.name, state.inclpath)
        if ffile:
            inc_files.append(ffile)
        else:
            raise NotFoundError(
                "Unable to resolve include file {}".format(i))
    [_HDR_PARSE((x, state)) for x in inc_files]
    # print("Parsing {}".format(inc_files))


class State(object):
    def __init__(self, literals, symtree, inclpath, incprocessed=False):
        self._literals = literals
        self._symtree = symtree
        self._inclpath = inclpath
        self._incprocessed = incprocessed
        self._mainsrc = None

    @property
    def literals(self):
        return self._literals

    @property
    def symtree(self):
        return self._symtree

    @property
    def inclpath(self):
        return self._inclpath

    @property
    def incprocessed(self):
        return self._incprocessed

    @incprocessed.setter
    def incprocessed(self, b):
        self._incprocessed = b

    @property
    def mainsrc(self):
        return self._mainsrc

    @mainsrc.setter
    def mainsrc(self, src):
        self._mainsrc = src


class SimpleBundle(object):
    """SimpleBundle contains src and resulting AST"""

    def __init__(self, inc_paths, srcfile, ast):
        self._inc_paths = inc_paths
        self._src_file = srcfile
        self._ast = ast

    @property
    def inc_paths(self):
        return self._inc_paths

    @property
    def src_file(self):
        return self._src_file

    @property
    def ast(self):
        return self._ast


class Bundle(SimpleBundle):
    """Main Bundle contains essential elements for action Handlers"""

    def __init__(self, incpath, srcfile, ast, state, outhandle, outfile):
        super().__init__(incpath, srcfile, ast)
        self._state = state
        self._out = outhandle
        self._out_file = outfile
        self._externs = None
        self._lambdas = []
        self._triple = None

    @property
    def literals(self):
        return self._state.literals

    @property
    def out(self):
        return self._out

    @property
    def out_file(self):
        return self._out_file

    @property
    def state(self):
        return self._state

    @property
    def externs(self):
        return self._externs

    @externs.setter
    def externs(self, externs):
        self._externs = externs

    @property
    def symtree(self):
        return self._state.symtree

    @symtree.setter
    def symtree(self, symtree):
        self._state.symtree = symtree

    @property
    def lambdas(self):
        return self._lambdas


class Handler(ABC):
    """Handler is pfoild abstract action handler class"""

    def __init__(self, bundle):
        self._bundle = bundle

    @property
    def bundle(self):
        return self._bundle

    @classmethod
    def handler_for(cls, action, bundle):
        if action == 'comp':
            return Comp(bundle)
        elif action == 'compr':
            return RTComp(bundle)
        elif action == 'hdr':
            return Hdr(bundle)
        elif action == 'diag':
            return Diag(bundle)
        else:
            return None

    @abstractmethod
    def validate(self):
        pass

    @abstractmethod
    def emit(self):
        pass

    def write(self, s):
        print(s, file=self.bundle.out)

    def complete(self):
        if self.bundle.out_file == 'stdout':
            pass
        else:
            self.bundle.out.flush()
            self.bundle.out.close()


class BaseComp(Handler):
    def __init__(self, bundle):
        super().__init__(bundle)
        self._ptree = None
        LOGGER.info("Initiating compile")

    @property
    def ptree(self):
        return self._ptree

    @ptree.setter
    def ptree(self, ptree):
        self._ptree = ptree

    def compute_include(self):
        include = self.bundle.ast.include
        # For each file include

        def resolve_hdr_file(fname):
            for p in self.bundle.inc_paths:
                hfile = util.file_exists(p, fname, 'defs')
                if hfile:
                    return hfile
            return None

        inc_files = []
        for i in include.value:
            ffile = resolve_hdr_file(i.value)
            if ffile:
                inc_files.append(ffile)
            else:
                raise NotFoundError(
                    "Unable to resolve include file {}".format(i.value))
        return inc_files

    def validate(self):
        # Transform ast to parse tree
        parse_tree = []
        self.bundle.ast.eval(self.bundle, parse_tree)
        self.ptree = parse_tree[0]
        # pprint.pprint(self.ptree)

    def emit(self):
        self.write(_std_comment)
        # Emit LLVM from parse tree
        mything = LlvmGen(self.ptree['base'][0].name)
        mything.emit(self.ptree, self.write)
        self.complete()


class Comp(BaseComp):
    def __init__(self, bundle):
        super().__init__(bundle)

    def validate(self):
        # Load system imports first
        # Load any imports
        # Extrapolate literals
        # Generate symbol tree
        # Refactor code and identifiers
        super().validate()
        pass


class RTComp(BaseComp):
    def __init__(self, bundle):
        super().__init__(bundle)
        # Does not set runtimes


class Hdr(Handler):
    """Header generation handler"""

    def __init__(self, bundle):
        super().__init__(bundle)
        LOGGER.info("Initiating header gen")

    def validate(self):
        self.write(_std_comment)

    def emit(self):
        """Hdr.emit() generates header content"""
        mod = self.bundle.ast
        self.write("module {}".format(mod.name))
        for d in mod.value:
            if type(d) is ast.Variable:
                self.write("var {} Any".format(d.name))
            elif type(d) is ast.Function:
                self.write("func {} [{}]".format(
                    d.name,
                    ' '.join([str(s.value) for s in d.arguments.value])))
        self.complete()


class Diag(Comp):
    def __init__(self, bundle):
        super().__init__(bundle)

    def _ast_trace(self, element, indent=1):
        """AST Hierarchy"""
        self.write("Trace: {}> {}".format(
            '-' * indent, element.__class__.__name__))
        if hasattr(element, 'value'):
            if type(element.value) is list:
                for i in element.value:
                    indent += 1
                    self._ast_trace(i, indent)
                    indent -= 1
            else:
                pass
        else:
            if type(element) is list:
                for l in element:
                    print("List => {}".format(l.value[0]))

    def _tree_walk(self, tree, indent=1):
        if type(tree) is list:
            self.write("Walk-l: {}> ({}): {}".format(
                '-' * indent, tree[0].name, tree[0].ptype))
            ttree = tree[0]
        elif isinstance(tree, ParseTree):
            self.write("Walk-p: {}> ({}): {}".format(
                '-' * indent, tree.name, tree.ptype))
            ttree = tree
        else:
            self.write("Walk-?: {}> {}".format(
                '-' * indent, tree.__class__.__name__))
            if type(tree) is dict:
                pprint.pprint(tree)
            self.write("Walk-l: {}> {}".format(
                '-' * indent, tree))
            ttree = ast.ParseTree(ExpressionType.IGNORE, [])

        if ttree.ptype in [
                ExpressionType.LET,
                ExpressionType.FUNCTION,
                ExpressionType.LAMBDA]:
            for p in ttree.pre:
                indent += 2
                self._tree_walk(p, indent)
                indent -= 2

        for i in ttree.exprs:
            indent += 1
            self._tree_walk(i, indent)
            indent -= 1

    def emit(self):
        print(self.bundle)
        self.write(_std_comment)
        self.write("AST Topology")
        self.write("------------")
        self._ast_trace(self.bundle.ast)
        self.write('\n')
        self.write("Parse Tree (lambdas)")
        self.write("--------------------")
        self._tree_walk(self.ptree['lambdas'])
        self.write('\n')
        self.write("Parse Tree (body)")
        self.write("-----------------")
        self._tree_walk(self.ptree['base'])
        self.write('\n')
        self.write("Literals")
        self.write("--------")
        self.write(pprint.pformat(self.ptree["literals"]))
        self.write('\n')
        self.write("Externs")
        self.write("-------")
        self.write(pprint.pformat(self.ptree['externs']))

        # self.write('\n')
        # self.write("Symbols")
        # self.write("-------")
        # for st in self.bundle.symtree.stack:
        #     self.write('\n')
        #     tstr = "Symbols for " + st.name
        #     self.write(tstr)
        #     self.write('-' * len(tstr))
        #     self.write(pprint.pformat(st.table))
        self.complete()
