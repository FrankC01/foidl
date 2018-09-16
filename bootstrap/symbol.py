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
from errors import SymbolException


LOGGER = logging.getLogger()


class SymbolTable(object):
    """SymbolTable exists for conmpilation and expression types
    """

    def __init__(self, short_name, long_name):
        self._name = short_name
        self._long_name = long_name
        self._table = {}

    @property
    def name(self):
        return self._name

    @property
    def long_name(self):
        return self._long_name

    @property
    def table(self):
        return self._table

    def locate(self, value):
        return self._table.get(value, None)

    def insert(self, value, reference):
        self._table[value] = reference


class SymbolTree(object):
    """Manages the tree of symbols and is preserved until emit"""

    def __init__(self, context, make_table=False, tree=None):
        self._context = context
        if tree:
            self._stack = tree
            self._current = tree[-1]
        else:
            self._stack = []
            self._current = None
        if make_table:
            first = SymbolTable(context)
            self._stack.append(first)
            self._current = first

    @property
    def context(self):
        return self._context

    @property
    def stack(self):
        return self._stack

    @property
    def current(self):
        return self._current

    def reverse_stack_locate(self, value):
        if type(value) is not str:
            LOGGER.warn("Argument passed {} is not a string".format(value))
            value = value.value
        LOGGER.debug("Symbol check for value {}".format(value))
        res = None
        for lvl in reversed(self.stack):
            LOGGER.debug(" Checking {}".format(lvl.name))
            res = lvl.locate(value)
            if res:
                LOGGER.debug("     Found {} in {}".format(value, lvl.name))
                break
        return res

    def resolve_symbol(self, value):
        LOGGER.debug("Asked to resolve {}".format(value))
        return self.reverse_stack_locate(value)

    def _var_override_msg(self, value, tref, sref):
        ttok = tref.token.getsourcepos()
        tsrc = os.path.split(tref.source)[1]
        ssrc = sref.source
        stok = sref.token.getsourcepos()
        return "symbol '{}' at {}:{}:{} hides {} defined in {}:{}:{}".format(
            value,
            tsrc,
            ttok.lineno,
            ttok.colno,
            sref.__class__.__name__,
            ssrc,
            stok.lineno,
            stok.colno)

    def register_symbol(self, value, reference):
        ref = self.reverse_stack_locate(value)
        if not ref:
            LOGGER.debug("Registering {} in {}".format(
                value, self.current.name))
            self.current.insert(value, reference)
        else:
            LOGGER.warn(self._var_override_msg(value, reference, ref))
            self.current.insert(value, reference)

    def push_scope(self, short_name, long_name):
        """Pushes new scope which becomes current"""
        LOGGER.debug("Pushing symbol table {}".format(short_name))
        table = SymbolTable(short_name, long_name)
        self._stack.append(table)
        self._current = table
        return table

    def pop_scope(self):
        """Removes and returns current scope"""
        if self._stack:
            table = self._stack.pop()
            LOGGER.debug("Pop {} from symtree".format(table.name))
            if self._stack:
                self._current = self._stack[-1]
            else:
                LOGGER.warn("Symbol Stack exhausted")
                self._current = None
        else:
            # Raise something
            raise SymbolException(
                "Attempting to pop empty Symbol Stack")
        return table
