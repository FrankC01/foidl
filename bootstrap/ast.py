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

from enums import WellKnowns
from abc import ABC, abstractmethod


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
        pass


class VarReference(FoidlReference):
    """Variable Symbol Reference"""
    pass


class FuncReference(FoidlReference):
    """FuncReference informs name, intern/extern, args"""

    def __init__(self, astmember, argcnt):
        super().__init__(astmember)
        self._argcnt = argcnt


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

    def eval(self, symtree, leader):
        # Register new level in symtree
        # Includes already handled
        # Walk children
        print("Module {} walk {}".format(self.name, self.value))
        symtree.push_scope(self.name, self.name)
        for c in self.value:
            c.eval(symtree, leader)


class Include(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

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

    def get_reference(self):
        return VarReference(self)

    def eval(self, symtree, leader):
        print("Variable {} eval {}".format(self.name, self.value))
        # Eval children
        # Register var name in symtable
        symtree.register_symbol(self.name, self.get_reference())
        pass


class Function(FoidlAst):
    def __init__(self, fname, value, token, src):
        super().__init__(token, src)
        self._arguments = value.pop(0)
        self._value = value
        self._name = fname

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

    @property
    def arguments(self):
        return self._arguments

    def get_reference(self):
        return FuncReference(self, self.arguments.elements())

    def eval(self, symtree, leader):
        print("Function {} eval {}".format(self.name, self.value))
        # Stack function and args in symtable
        symtree.push_scope(self.name, self.name)
        symtree.register_symbol(self.name, self.get_reference())
        for a in self.arguments.value:
            symtree.register_symbol(
                a.name,
                FuncArgReference(a))
        # Eval children
        for e in self.value:
            e.eval(symtree, leader)

        # Refactor
        # Pop stack
        symtree.pop_scope()
        # Register function symbol
        symtree.register_symbol(self.name, self.get_reference())
        pass


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
        print("Expressions")
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
        self.value = value
        self.call_site = csite[:-1]
        self.call_site = WellKnowns.get(self.call_site, self.call_site)

    def eval(self, symtree, leader):
        print("Function call {}".format(self.call_site))
        print(symtree.resolve_symbol(self.call_site))
        for e in self.value:
            e.eval(symtree, leader)


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
        print(symtree.resolve_symbol(self.name))
        print("Symbol {}".format(self.name))
        pass
