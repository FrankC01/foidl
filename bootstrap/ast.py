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

from abc import ABC, abstractmethod


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
    def eval(self):
        pass


class CompilationUnit(FoidlAst):
    """Scoped AST compilation unit value"""

    def __init__(self, value):
        self.value = value

    def eval(self):
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

    def eval(self):
        pass


class Include(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    def eval(self):
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

    def eval(self):
        pass


class Function(FoidlAst):
    def __init__(self, fname, value, token, src):
        super().__init__(token, src)
        self._value = value
        self._name = fname

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

    def eval(self):
        pass


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

    def eval(self):
        print("CALL {} with {}".format(self.call_site, self.value))
        for c in self.value:
            c.eval()


class Literal(FoidlAst):
    def __init__(self, ltype, lvalue, token, src):
        super().__init__(token, src)
        self.type = ltype
        self.value = lvalue

    def eval(self):
        print("Listeral {} => {}".format(self.type, self.value))


class Symbol(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    def eval(self):
        pass


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
