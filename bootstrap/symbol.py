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
import enums
from abc import ABC, abstractmethod


LOGGER = logging.getLogger()


class SymbolException(Exception):
    pass


class BaseReference(ABC):
    """BaseReference informs type, value, intern/extern, etc"""

    def __init__(self, token):
        self._token = token

    @property
    def token(self):
        return self._token


class LiteralReference(BaseReference):
    """Literal value reference"""
    pass


class VarReference(BaseReference):
    """Variable Symbol Reference"""
    pass


class FuncArgReference(BaseReference):
    """Function Argument Symbol Reference"""
    pass


class FuncReference(BaseReference):
    """FuncReference informs name, intern/extern, arg-count"""
    pass


class LambdaReference(BaseReference):
    """Lambda Reference"""
    pass


class LetArgReference(BaseReference):
    """Let Argument Symbol Reference"""
    pass


class LetResReference(BaseReference):
    """Let Result Symbol Reference"""
    pass


class MatchResReference(BaseReference):
    """Match Result Symbol Reference"""
    pass


class SymbolTable(object):
    """SymbolTable exists for conmpilation and expression types
    """

    def __init__(self, name, token):
        self._name = name
        self._token = token
        self._table = {}

    @property
    def name(self):
        return self._name

    @property
    def token(self):
        return self._token

    def locate(self, value):
        return self._table.get(value, None)

    def insert(self, value, reference):
        if not self.locate(value):
            self._table[value] = reference
        else:
            raise SymbolException(
                "{} already exists as {}".format(
                    value, self._table[value]))


class SymbolTree(object):
    """Manages the tree of symbols and is preserved until emit"""

    def __init__(self, context, token, tree=None):
        self._context = context
        if tree:
            self._stack = tree
            self._current = tree[-1]
        else:
            self._stack = []
            self._current = None
        first = SymbolTable(context, token)
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

    def push_scope(self, reference, token):
        """Pushes new scope which becomes current"""
        table = SymbolTable(reference, token)
        self._stack.append(table)
        self._current = table
        return table

    def pop_scope(self):
        """Removes and returns current scope"""
        if self._stack:
            table = self._stack.pop()
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
