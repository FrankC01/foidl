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

import logging

import ast
from enums import CollTypes

LOGGER = logging.getLogger()


class HParser(object):
    def __init__(self, mlexer, input):
        self._input = input
        self._tstrm = None
        self._state = None
        self._values = []
        self._symboltypes = ['SYMBOL', 'SYMBOL_PRED', 'SYMBOL_BANG']

    @property
    def symboltypes(self):
        return self._symboltypes

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, state):
        self._state = state

    @property
    def tstrm(self):
        return self._tstrm

    @tstrm.setter
    def tstrm(self, tstrm):
        self._tstrm = tstrm

    @property
    def values(self):
        return self._values

    @property
    def input(self):
        return self._input

    def get_parser(self):
        return self

    def get_symbol(self, tstrm):
        tk = tstrm.next()
        if tk.gettokentype() in self.symboltypes:
            return True, tk, ast.Symbol(
                tk.getstr(),
                tk,
                self.input)
        else:
            return False, tk, None

    def get_symbols_list(self, tstrm, obin=None):
        slist = []
        ob = obin if obin else tstrm.next()
        if ob.getstr() == '[':
            while True:
                h, tk, x = self.get_symbol(tstrm)
                if h:
                    slist.append(x)
                elif tk.getstr() == ']':
                    break
                else:
                    print("Something bad")
        else:
            print("Way something bad")
        return slist

    def get_var(self, vtok, tstrm):
        h, tk, s = self.get_symbol(tstrm)
        if tk.getstr() != 'private' or tk.getstr() != ':private':
            # print("Processing variable {}".format(tk))
            self.values.append(
                ast.Variable(
                    ast.VarHeader(s, tk, self.input),
                    [ast.NIL], tk, self.input))

    def get_func(self, ftok, tstrm):
            h, tk, funcs = self.get_symbol(tstrm)
            if tk.getstr() != 'private' or tk.getstr() != ':private':
                # print("Processing function {}".format(tk))
                args = self.get_symbols_list(tstrm)
                argv = None
                if args:
                    argv = ast.SymbolList(args)
                else:
                    argv = ast.EmptyCollection(
                        CollTypes.LIST, [], ftok, self.input)
                self.values.append(
                    ast.Function(
                        ast.FuncHeader(
                            funcs, argv, ftok, self.input), None, self.input))

    def get_decl(self, tstrm):
        hits = {
            'var': self.get_var,
            'func': self.get_func}

        while True:
            try:
                tk = tstrm.next()
                fn = hits.get(tk.getstr(), None)
                if fn:
                    fn(tk, tstrm)
            except StopIteration:
                break

    def module(self, tstrm):
        modsym = None
        nextone = tstrm.next()
        if nextone.getstr() == 'module':
            _, tk, modsym = self.get_symbol(tstrm)
            self.get_decl(tstrm)
            return ast.Module(modsym, self.values, tk, self.input)
        else:
            print('Something bad {}'.format(modsym))
            return

    def parse(self, tstrm=None, state=None):
        self.tstrm = tstrm
        self.state = state
        if tstrm:
            module = self.module(self.tstrm)
            return ast.CompilationUnit([module])
        else:
            return None
