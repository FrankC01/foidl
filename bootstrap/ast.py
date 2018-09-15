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
import errors
from enums import WellKnowns
from abc import ABC, abstractmethod

LOGGER = logging.getLogger()


class FoidlReference(ABC):
    """FoidlReference informs type, value, intern/extern, etc"""

    def __init__(self, astmember):
        self._token = astmember.token
        self._source = astmember.source

    def set_individual(self, token, src):
        self._token = token
        self._source = src

    @property
    def token(self):
        return self._token

    @property
    def source(self):
        return self._source


class LiteralReference(FoidlReference):
    """Literal value reference"""

    def __init__(self, id, token, src):
        super().set_individual(token, src)
        self._literal_type = token.gettokentype()
        self._literal_value = token.getstr()
        self._identifier = id

    @property
    def identifier(self):
        return self._identifier

    @property
    def literal_type(self):
        return self._literal_type

    def eval(self, symtable, leader):
        print("Literal {}".format(self.literal_type))
        leader.append(self)


class VarReference(FoidlReference):
    """Variable Symbol Reference"""
    pass


class FuncReference(FoidlReference):
    """FuncReference informs name, intern/extern, args"""

    def __init__(self, astmember, argcnt):
        super().__init__(astmember)
        self._argcnt = argcnt

    @property
    def argcnt(self):
        return self._argcnt


class FuncArgReference(FoidlReference):
    """Function Argument Symbol Reference"""


class LambdaReference(FoidlReference):
    """Lambda Reference"""
    pass


class LetArgReference(FoidlReference):
    """Let Argument Symbol Reference"""
    pass


class LetResReference(FoidlReference):
    """Let Result Symbol Reference"""
    pass


class MatchResReference(FoidlReference):
    """Match Result Symbol Reference"""
    pass


class FoidlAst(ABC):
    """Base abstract ast class"""

    def __init__(self, token, src='unknown'):
        self._token = token
        self._source = src

    @property
    def token(self):
        return self._token

    @property
    def source(self):
        return self._source

    @abstractmethod
    def eval(self, symtree, leader):
        pass


class CompilationUnit(FoidlAst):
    """Scoped AST compilation unit value"""

    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        """Traverse the tree"""
        self.value[0].eval()

    def module(self):
        return self.value[0]


class Module(FoidlAst):
    def __init__(self, mname, value, token, src):
        super().__init__(token, src)
        self._name = mname
        self._include = None
        if type(value[0]) == Include:
            self._include = value.pop(0)
        else:
            self._include = Include([], token, src)
        self.value = value

    @property
    def include(self):
        return self._include

    @property
    def name(self):
        return self._name.value

    def eval(self, symtree, literals, ptree):
        # Register new level in symtree
        # Includes already handled
        # Walk children
        print("Module {} walk {}".format(self.name, self.value))
        symtree.push_scope(self.name, self.name)
        children = []
        for c in self.value:
            c.eval(symtree, children)
        ptree.append({
            "name": self.name,
            "type": 'module',
            "literals": literals,
            "decls": children})


class Include(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    def eval(self, symtree, leader):
        pass


class VarHeader(FoidlAst):
    def __init__(self, vname, token, src):
        super().__init__(token, src)
        self._name = vname

    @property
    def name(self):
        return self._name

    def get_reference(self):
        return VarReference(self)

    def eval(self, symtree, leader):
        pass


class Variable(FoidlAst):
    def __init__(self, vname, value, token, src):
        super().__init__(token, src)
        self._value = value
        self._name = vname

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

    def eval(self, symtree, leader):
        print("Variable {} eval {}".format(self.name, self.value))
        expr = []
        # Eval children
        for e in self.value:
            e.eval(symtree, expr)
        leader.append({
            "name": self.name,
            'type': 'var',
            'expr': expr})


class FuncHeader(FoidlAst):
    def __init__(self, fname, args, token, src):
        super().__init__(token, src)
        self._name = fname
        self._arguments = args

    @property
    def name(self):
        return self._name

    @property
    def arguments(self):
        return self._arguments

    def get_reference(self):
        return FuncReference(self, self.arguments.elements())

    def eval(self, symtree, leader):
        pass


class Function(FoidlAst):
    def __init__(self, fhdr, value, src):
        super().__init__(fhdr.token, src)
        self._name = fhdr.name
        self._arguments = fhdr.arguments
        self._value = value

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

    @property
    def arguments(self):
        return self._arguments

    def eval(self, symtree, leader):
        # print("Function {} eval {}".format(self.name, self.value))
        # Stack args in symtable
        symtree.push_scope(self.name, self.name)
        for a in self.arguments.value:
            symtree.register_symbol(
                a.name,
                FuncArgReference(a))
        # Eval children
        expr = []
        for e in self.value:
            e.eval(symtree, expr)
        leader.append({
            "name": self.name,
            'type': 'func',
            'expr': expr})
        # Refactor
        # Pop stack
        symtree.pop_scope()


class CollectionAst(FoidlAst):
    @abstractmethod
    def elements(self):
        pass


class SymbolList(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, symtree, leader):
        print("Symbol List {}".format(self.value))
        for c in self.value:
            c.eval()


class LetPairs(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, symtree, leader):
        print("Let pairs {}".format(self.value))
        for c in self.value:
            c.eval()


class MatchPairs(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, symtree, leader):
        print("Match pairs {}".format(self.value))
        for c in self.value:
            c.eval()


class EmptyCollection(FoidlAst):
    def __init__(self, etype):
        self.type = etype
        self.value = []

    def elements(self):
        return 0

    def eval(self, symtree, leader):
        print("Empty collection type {}".format(self.type))


class Collection(CollectionAst):
    def __init__(self, ctype, value):
        self.value = value
        self.type = ctype

    def elements(self):
        return len(self.value)

    def eval(self, symtree, leader):
        print("Collection {} value {}".format(self.type, self.value))
        for c in self.value:
            c.eval()


class ExpressionList(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Expression List {}".format(self.value))
        for c in self.value:
            c.eval()


class Expressions(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        # print("Expressions")
        for e in self.value:
            e.eval(symtree, leader)


class Expression(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        for e in self.value:
            e.eval(symtree, leader)


class Group(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Group value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class Let(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Let value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class Match(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Match value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class Lambda(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Lambda value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class Partial(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, symtree, leader):
        print("Partial value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class If(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self.value = value

    def eval(self, symtree, leader):
        print("If value {}".format(self.value))
        for e in self.value:
            e.eval(symtree, leader)


class FunctionCall(FoidlAst):
    def __init__(self, csite, value, token, src):
        super().__init__(token, src)
        if value and type(value[0]) is Expressions:
            self.value = value[0].value
        else:
            self.value = value
        self.call_site = csite

    @classmethod
    def re_factor(cls, csite, value, token, src, state):
        bvalue = value[0].value if value else value
        call_site = WellKnowns.get(csite[:-1], csite[:-1])
        cref = state.symtree.resolve_symbol(call_site)
        if cref.argcnt != len(bvalue):
            if len(bvalue) < cref.argcnt:
                raise errors.FunctionCall(
                    "Function {} expects {} args, have {}".format(
                        call_site,
                        cref.argcnt,
                        len(bvalue)))
            else:
                tlist = bvalue[0:cref.argcnt]
                res = bvalue[cref.argcnt:len(bvalue)]
                fc = FunctionCall(call_site, tlist, token, src)
                fl = [fc]
                fl.extend(res)
                return fl
        else:
            return FunctionCall(call_site, value, token, src)

    def eval(self, symtree, leader):
        print("Processing function call {}".format(self.call_site))
        # print(symtree.resolve_symbol(self.call_site))
        args = []
        for e in self.value:
            e.eval(symtree, args)
        res = None
        leader.append({
            'name': self.call_site,
            'type': 'call',
            'loc': self.token.getsourcepos(),
            'expr': args})
        if res:
            leader.append(res)


class Symbol(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    @property
    def name(self):
        return self._value

    def eval(self, symtree, leader):
        print("Processing symbol")
        leader.append(symtree.resolve_symbol(self.name))
