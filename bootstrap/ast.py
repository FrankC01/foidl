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

import enum
from abc import ABC, abstractmethod


@enum.unique
class CollTypes(enum.Enum):
    VECTOR = enum.auto()
    LIST = enum.auto()
    MAP = enum.auto()
    SET = enum.auto()


def ast_trace(el, indent=1):
    print("Trace: {}> {}".format(
        '-' * indent, el.__class__.__name__))
    if hasattr(el, 'value'):
        if type(el.value) is list:
            for i in el.value:
                indent += 1
                ast_trace(i, indent)
                indent -= 1
        else:
            pass
    else:
        pass


class AstState():
    def __init__(self):
        self.literals = {}

    def process_literal(self, literal):
        """Literals are grouped and reused as needed"""
        x = self.literals.get(literal.type)
        idx = None
        if not x:
            idx = literal.type + "_0"
            self.literals[literal.type] = {literal.value: idx}
        else:
            vmap = self.literals[literal.type]
            idx = literal.type + "_" + str(len(vmap))
            ydx = vmap.get(literal.value)
            if not ydx:
                vmap[literal.value] = idx
            else:
                idx = ydx
        literal.value = idx
        return literal


class FoidlAst(ABC):
    """Base abstract ast class"""

    @abstractmethod
    def eval(self):
        pass


class CompilationScope(FoidlAst):
    """Scoped AST compilation unit value"""

    def __init__(self):
        self.literals = {}
        self.value = None

    def build_unit(self, module):
        """From parse, attach the module as the value"""
        self.value = module
        return self

    def eval(self):
        """Traverse the tree"""
        self.value[0].eval()


class Module(FoidlAst):
    def __init__(self, mname, value):
        self.value = value
        self.name = mname

    def eval(self):
        print("Module *{}* value {}".format(self.name.value, self.value))
        for c in self.value:
            c.eval()


class Include(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Include value {}".format(self.value))
        for c in self.value[1:]:
            c.eval()


class Variable(FoidlAst):
    def __init__(self, vname, value):
        self.value = value
        self.name = vname

    def eval(self):
        print("VAR *{}* value {}".format(self.name.value, self.value))
        for c in self.value:
            c.eval()


class Function(FoidlAst):
    def __init__(self, fname, value):
        self.value = value
        self.name = fname

    def eval(self):
        print("FUNCTION *{}* value {}".format(self.name.value, self.value))
        for c in self.value:
            c.eval()


class SymbolList(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Symbol List {}".format(self.value))
        for c in self.value:
            c.eval()


class LetPairs(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Let pairs {}".format(self.value))
        for c in self.value:
            c.eval()


class MatchPairs(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Match pairs {}".format(self.value))
        for c in self.value:
            c.eval()


class EmptyCollection(FoidlAst):
    def __init__(self, etype):
        self.type = etype

    def eval(self):
        print("Empty collection type {}".format(self.type))


class Collection(FoidlAst):
    def __init__(self, ctype, value):
        self.value = value
        self.type = ctype

    def eval(self):
        print("Collection {} value {}".format(self.type, self.value))
        for c in self.value:
            c.eval()


class ExpressionList(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Expression List {}".format(self.value))
        for c in self.value:
            c.eval()


class Expressions(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Expressions {}".format(self.value))
        for c in self.value:
            c.eval()


class Expression(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Expr value {}".format(self.value))
        for c in self.value:
            c.eval()


class Group(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Group value {}".format(self.value))
        for c in self.value:
            c.eval()


class Let(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Let value {}".format(self.value))
        for c in self.value:
            c.eval()


class Match(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Match value {}".format(self.value))
        for c in self.value:
            c.eval()


class Lambda(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Lambda value {}".format(self.value))
        for c in self.value:
            c.eval()


class Partial(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Partial value {}".format(self.value))
        for c in self.value:
            c.eval()


class FunctionCall(FoidlAst):
    def __init__(self, csite, value):
        self.value = value
        self.call_site = csite
        print("Registering {}".format(csite))

    def eval(self):
        print("CALL {} with {}".format(self.call_site, self.value))
        for c in self.value:
            c.eval()


class Literal(FoidlAst):
    def __init__(self, ltype, lvalue):
        self.type = ltype
        self.value = lvalue

    def eval(self):
        print("Listeral {} => {}".format(self.type, self.value))


class Symbol(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Symbol {}".format(self.value))


class Real(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self):
        print("Real {}".format(self.value))
