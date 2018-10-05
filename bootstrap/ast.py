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
from abc import ABC, abstractmethod
from functools import singledispatch, update_wrapper

from rply import Token

import errors
from enums import WellKnowns, CollTypes, ExpressionType
from ptree import *
from refactor import refactor_lambda

LOGGER = logging.getLogger()


def methdispatch(func):
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


class FoidlReference(ABC):
    """FoidlReference informs type, value, intern/extern, etc"""

    def __init__(self, astmember):
        self._token = astmember.token if astmember else None
        self._source = astmember.source if astmember else None

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
        base = self.source.split("/")[-1].split('.')[0]
        self._identifier = base + "_" + self.literal_type + "_" + id

    @property
    def identifier(self):
        return self._identifier

    @property
    def name(self):
        return self.identifier

    @property
    def value(self):
        return self.token.getstr()

    @property
    def literal_type(self):
        return self.token.gettokentype()

    def eval(self, bundle, leader):
        leader.append(
            ParseLiteral(
                ExpressionType.LITERAL,
                [self],
                self.token,
                self.value))


class VarReference(FoidlReference):
    """Variable Symbol Reference"""

    def __init__(self, astmember, nm):
        super().__init__(astmember)
        self._name = nm
        self._exprs = None

    @property
    def name(self):
        return self._name

    @property
    def exprs(self):
        return self._exprs

    @exprs.setter
    def exprs(self, exprs):
        self._exprs = exprs


class FuncReference(FoidlReference):
    """FuncReference informs name, intern/extern, args"""

    def __init__(self, astmember, name, argcnt):
        super().__init__(astmember)
        self._argcnt = argcnt
        self._name = name

    @property
    def argcnt(self):
        return self._argcnt

    @property
    def name(self):
        return self._name


class FuncArgReference(FoidlReference):
    """Function Argument Symbol Reference"""

    def __init__(self, astmember, argname, argpos):
        super().__init__(astmember)
        self._argname = argname
        self._argpos = argpos

    @property
    def argname(self):
        return self._argname

    @argname.setter
    def argname(self, argname):
        self._argname = argname

    @property
    def argpos(self):
        return self._argpos

    @argpos.setter
    def argpos(self, argpos):
        self._argpos = argpos

    @classmethod
    def replicate(cls, otherref):
        far = FuncArgReference(None, otherref.argname, 0)
        far.set_individual(otherref.token, otherref.source)
        return far

    def __repr__(self):
        return "FuncArgRef({}:{})".format(self.argname, self.argpos)


class LambdaReference(FoidlReference):
    """Lambda Reference"""

    def __init__(self, astmember, ident, acnt):
        super().__init__(astmember)
        self._ident = ident
        self._argcnt = acnt

    @property
    def ident(self):
        return self._ident

    @property
    def argcnt(self):
        return self._argcnt


class LetArgReference(FoidlReference):
    """Let Argument Symbol Reference"""

    def __init__(self, astmember):
        super().__init__(astmember)
        self._name = astmember.value
        self._ident = None
        self._ptr = None

    @property
    def name(self):
        return self._name

    @property
    def argname(self):
        return self._name

    @property
    def ident(self):
        return self._ident

    @ident.setter
    def ident(self, ident):
        self._ident = ident

    @property
    def ptr(self):
        return self._ptr

    @ptr.setter
    def ptr(self, pointer):
        self._ptr = pointer

    def __repr__(self):
        return "LetArgRef({}:ptr {})".format(self.argname, self.ptr)


class ResultReference(FoidlReference):
    def __init__(self, astmember):
        super().__init__(astmember)
        self._name = astmember.name
        self._ident = None
        self._ptr = None

    @property
    def name(self):
        return self._name

    @property
    def argname(self):
        return self.name

    @property
    def ident(self):
        return self._ident

    @ident.setter
    def ident(self, ident):
        self._ident = ident

    @property
    def ptr(self):
        return self._ptr

    @ptr.setter
    def ptr(self, pointer):
        self._ptr = pointer


class LetResReference(ResultReference):
    """Let Result Symbol Reference"""

    def __init__(self, astmember):
        super().__init__(astmember)

    def __repr__(self):
        return "LetResRef({}:ptr {})".format(self.argname, self.ptr)


class MatchResReference(ResultReference):
    """Match Result Symbol Reference"""

    def __init__(self, astmember):
        super().__init__(astmember)

    def __repr__(self):
        return "MatchResRef({}:ptr {})".format(self.argname, self.ptr)


class MatchExprReference(ResultReference):
    """Match Result Symbol Reference"""

    def __init__(self, astmember):
        super().__init__(astmember)

    def __repr__(self):
        return "MatchExprRef({}:ptr {})".format(self.argname, self.ptr)


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
    def eval(self, bundle, leader):
        pass

    def extern_symbol(self, cname, bundle, source):
        sym = bundle.symtree.resolve_symbol(cname)
        if sym:
            if sym.source != self.source:
                if not bundle.externs.get(cname, None):
                    bundle.externs[cname] = sym
            return sym
        else:
            raise errors.SymbolException(
                "{}:{} unresolved symbol {}".format(
                    self.token.getsourcepos().lineno,
                    self.token.getsourcepos().colno,
                    self.name))


class CompilationUnit(FoidlAst):
    """Scoped AST compilation unit value"""

    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        """Traverse the tree"""
        self.value[0].eval()

    def module(self):
        return self.value[0]


class Symbol(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self.value = value

    @property
    def name(self):
        return self.value

    def eval(self, bundle, leader):
        LOGGER.info("Symbol processing for {}".format(self.name))
        LOGGER.info(self.name)
        # sym = self.extern_symbol(cname, call_site.name, self.source)
        sym = bundle.symtree.resolve_symbol(self.name)
        if sym:
            if self.source != sym.source:
                if not bundle.externs.get(self.name, None):
                    bundle.externs[self.name] = sym
            leader.append(ParseSymbol(
                ExpressionType.SYMBOL_REF,
                [sym],
                self.token,
                self.name))
        else:
            raise errors.SymbolException(
                "{}:{} unresolved symbol {}".format(
                    self.token.getsourcepos().lineno,
                    self.token.getsourcepos().colno,
                    self.name))


class Module(FoidlAst):
    """Module AST is the top node in tree

    During evluation it establishes externs, literals and
    lambdas as part of the generation of Parse Tree """

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

    def eval(self, bundle, ptree):
        # Push module scope for duration of evaluation
        bundle.symtree.push_scope(self.name, self.name)
        # Add references to nil, true, false
        bundle.externs = {
            "nil": bundle.symtree.resolve_symbol("nil"),
            "eq": bundle.symtree.resolve_symbol("eq"),
            "true": bundle.symtree.resolve_symbol("true"),
            "false": bundle.symtree.resolve_symbol("false")
        }
        # Walk children
        children = []
        for c in self.value:
            c.eval(bundle, children)
        ptree.append({
            "base": [ParseTree(
                ExpressionType.MODULE,
                children,
                self.token,
                self.name)],
            "externs": bundle.externs,
            "literals": bundle.literals,
            "lambdas": bundle.lambdas})


class Include(FoidlAst):
    """Include AST node"""

    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    def eval(self, bundle, leader):
        pass


class VarHeader(FoidlAst):
    """Transitory in creation of true Variable AST node"""

    def __init__(self, vname, token, src):
        super().__init__(token, src)
        self._name = vname
        self._reference = None

    @property
    def name(self):
        return self._name

    @property
    def reference(self):
        return self._reference

    def get_reference(self):
        self._reference = VarReference(self, self.name.value)
        return self._reference

    def eval(self, bundle, leader):
        pass


class Variable(FoidlAst):
    """Variable AST captures identity and associated expression"""

    def __init__(self, vhdr, value, token, src):
        super().__init__(token, src)
        self._name = vhdr.name
        self._vref = vhdr.reference
        if type(value[0]) is list:
            value = value[0]
        if len(value) != 1:
            raise errors.ParseError(
                "variable '{}' at line {} has more than 1 expression".format(
                    vname.value,
                    token.getsourcepos().lineno))
        self._value = value

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

    def eval(self, bundle, leader):
        expr = []
        # Eval children
        for e in self.value:
            e.eval(bundle, expr)
        self._vref.exprs = expr
        leader.append(
            ParseTree(
                ExpressionType.VARIABLE,
                expr,
                self.token,
                self.name))


class FuncHeader(FoidlAst):
    """Transitory in creation of true Function AST node"""

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
        return FuncReference(self, self.name.value, self.arguments.elements())

    def eval(self, bundle, leader):
        pass


class Function(FoidlAst):
    def __init__(self, fhdr, value, src):
        super().__init__(fhdr.token, src)
        self._name = fhdr.name
        self._arguments = fhdr.arguments
        self.value = value if value else [_NIL]

    @property
    def name(self):
        return self._name.value

    @property
    def arguments(self):
        return self._arguments

    def eval(self, bundle, leader):
        # Stack args in symtable
        bundle.symtree.push_scope(self.name, self.name)
        i = 0
        argref = []
        for a in self.arguments.value:
            ar = FuncArgReference(a, a.name, i)
            bundle.symtree.register_symbol(a.name, ar)
            argref.append(ar)
            i += 1

        # Eval children
        expr = []
        for e in self.value:
            if type(e) is Expressions:
                for z in e.value:
                    LOGGER.info("Processing {}".format(z))
                    z.eval(bundle, expr)
            else:
                LOGGER.info("Processing {}".format(e))
                e.eval(bundle, expr)
        leader.append(
            ParseFunction(
                ExpressionType.FUNCTION,
                expr,
                self.token,
                self.name,
                pre=argref))
        # Pop stack
        bundle.symtree.pop_scope()


class CollectionAst(FoidlAst):
    @abstractmethod
    def elements(self):
        pass


class SymbolList(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, bundle, leader):
        for c in self.value:
            c.eval(bundle, leader)


class EmptyCollection(CollectionAst):
    def __init__(self, etype, value, token, src):
        super().__init__(token, src)
        self.type = etype
        self.value = value

    def elements(self):
        return 0

    @classmethod
    def generate(cls, ct, t, src):
        if ct is CollTypes.LIST:
            v = Symbol('empty_list', t, src)
        elif ct is CollTypes.VECTOR:
            v = Symbol('empty_vector', t, src)
        elif ct is CollTypes.SET:
            v = Symbol('empty_set', t, src)
        else:
            v = Symbol('empty_map', t, src)
        return EmptyCollection(ct, [v], t, src)

    def eval(self, bundle, leader):
        members = []
        for e in self.value:
            e.eval(bundle, members)

        leader.append(
            ParseEmpty(
                ExpressionType.EMPTY_COLLECTION,
                members,
                self.token,
                res=self.type))


class Collection(CollectionAst):
    def __init__(self, ctype, value, token, src):
        super().__init__(token, src)
        self.value = value
        self.type = ctype
        self.ident = ctype.name + "_" \
            + str(token.getsourcepos().lineno) + "_" \
            + str(token.getsourcepos().colno)

    def elements(self):
        return len(self.value)

    def eval(self, bundle, leader):
        if self.type is CollTypes.VECTOR:
            ccls = ParseVector
            ctyp = ExpressionType.VECTOR_COLLECTION
        elif self.type is CollTypes.LIST:
            ccls = ParseList
            ctyp = ExpressionType.LIST_COLLECTION
        elif self.type is CollTypes.MAP:
            ccls = ParseMap
            ctyp = ExpressionType.MAP_COLLECTION
        elif self.type is CollTypes.SET:
            ccls = ParseSet
            ctyp = ExpressionType.SET_COLLECTION
        else:
            raise errors.SyntaxError(
                "Unknown collection type {}".format(self.type))

        members = []
        for c in self.value:
            c.eval(bundle, members)

        leader.append(
            ccls(
                ctyp,
                members,
                self.token,
                self.ident))


class ExpressionList(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        for c in self.value:
            c.eval(bundle, leader)


class Expressions(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        for e in self.value:
            e.eval(bundle, leader)


class Expression(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        for e in self.value:
            e.eval(bundle, leader)


class Group(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._ident = "grp_" \
            + str(token.getsourcepos().lineno) \
            + "_" + str(token.getsourcepos().colno)
        self.value = value

    @property
    def ident(self):
        return self._ident

    def eval(self, bundle, leader):
        bundle.symtree.push_scope(self.ident, self.ident)

        exprs = []
        for e in self.value:
            e.eval(bundle, exprs)
        bundle.symtree.pop_scope()
        if exprs:
            leader.append(
                ParseGroup(
                    ExpressionType.GROUP,
                    exprs,
                    self.token,
                    self.ident))


class LetPairs(CollectionAst):
    def __init__(self, value):
        self.value = value
        self._prefix = None

    def elements(self):
        return len(self.value)

    @property
    def prefix(self):
        return self._prefix

    @prefix.setter
    def prefix(self, prefix):
        self._prefix = prefix

    def eval(self, bundle, leader):
        # Put symbols in table and evaluate expression
        for i, j in zip(*[iter(self.value)] * 2):
            expr = []
            j.eval(bundle, expr)
            ident = self.prefix + "_" + i.value
            lar = LetArgReference(i)
            lar.ident = ident
            bundle.symtree.register_symbol(i.value, lar)
            leader.append(
                ParseLetPair(
                    ExpressionType.LET_ARG_PAIR,
                    expr,
                    res=lar,
                    name=ident))


class Let(FoidlAst):
    def __init__(self, symbol, lpairs, expr, token, src):
        super().__init__(token, src)
        self._id = symbol
        self._letpairs = lpairs

        if type(lpairs) is not EmptyCollection:
            if len(self._letpairs.value) % 2:
                raise errors.ParseError(
                    "{}:{} un-even let pairs".format(
                        token.getsourcepos().lineno,
                        token.getsourcepos().colno))

        self.value = expr if type(expr[0]) is not list else expr[0]

        if len(self.value) > 1:
            raise errors.ParseError(
                "Let expects 1 expression in body, found {}".format(
                    len(self.value)))

    @property
    def letpairs(self):
        return self._letpairs

    @classmethod
    def generate(cls, p, src):
        t = p.pop(0)    # Get the 'LET' token
        # Identify symbol if exists
        if type(p[0]) is Symbol:
            symbol = p.pop(0)
        else:
            sp = t.getsourcepos()
            lsym = "let_" + str(sp.lineno) + "_" + str(sp.colno)
            symbol = Symbol(
                lsym,
                Token(
                    "LET_RES",
                    lsym,
                    t.getsourcepos()), src)
        # Extrapolate letpairs
        lbindex = p.index(Token('LBRACKET', '['))
        rbindex = 1 + p.index(Token('RBRACKET', ']'))

        letpairs = [lp for lp in p[lbindex:rbindex]
                    if type(lp) is not Token]
        if letpairs:
            letpairs = letpairs[0]
        else:
            letpairs = EmptyCollection.generate(
                CollTypes.LIST, t, src)
            # letpairs = EmptyCollection(CollTypes.LIST, [], t, src)

        # Remove letpair range
        del p[lbindex:rbindex]

        # If value at 0 is list, get inner list
        if type(p[0]) is list:
            value = p[0]
        else:
            value = p

        # if too many elements, structure expressions
        # so [<let> <residuals>]
        if len(value) > 1:
            mylet = Let(symbol, letpairs, [value.pop(0)], t, src)
            value.insert(0, mylet)
            return value
        else:
            return Let(symbol, letpairs, value, t, src)

    def eval(self, bundle, leader):
        # Push Let scope on bundle
        bundle.symtree.push_scope(self._id, self._id)
        # Generate unique_id
        unique_id = "let_" + str(self.token.getsourcepos().lineno) \
            + "_" + str(self.token.getsourcepos().colno)
        # Register any letpair vars in table
        preblock = []
        self._letpairs.prefix = unique_id
        self._letpairs.eval(bundle, preblock)
        # Process
        expr = []
        for e in self.value:
            e.eval(bundle, expr)
        # Pop scope
        bundle.symtree.pop_scope()
        # Create result reference
        lres = LetResReference(self._id)
        lres.ident = "letres_" + str(self.token.getsourcepos().lineno) \
            + "_" + str(self.token.getsourcepos().colno)
        # Register id
        bundle.symtree.register_symbol(self._id.value, lres)
        # Set let in leader
        leader.append(
            ParseLet(
                ExpressionType.LET,
                expr,
                self.token,
                # self._id.value,
                unique_id,
                lres,
                preblock))


class MatchPair(FoidlAst):
    _singular = [
        Symbol, LiteralReference, FuncArgReference, LetResReference,
        MatchResReference, MatchExprReference]

    def __init__(self, value, token, src, default=False):
        super().__init__(token, src)
        self._value = value
        self._default = default
        self._prefix = None
        print("Match Pair {}".format(value))

    @property
    def value(self):
        return self._value

    @property
    def default(self):
        return self._default

    @property
    def prefix(self):
        return self._prefix

    @prefix.setter
    def prefix(self, prefix):
        self._prefix = prefix

    def eval(self, bundle, leader):
        expr = []
        self.value[1].eval(bundle, expr)
        pre = []
        mpairname = "match_pair_" + \
            str(self.token.getsourcepos().lineno) + \
            "_" + str(self.token.getsourcepos().colno)
        if self.default:
            leader.append(
                ParseMatchDefault(
                    ExpressionType.MATCH_PAIR,
                    expr,
                    self.token,
                    mpairname,
                    pre=None))
        else:
            self.value[0].eval(bundle, pre)
            if type(self.value[0]) in self._singular:
                leader.append(
                    ParseMatchEqual(
                        ExpressionType.MATCH_PAIR,
                        expr,
                        self.token,
                        mpairname,
                        pre=pre))
            else:
                print("Eval pre on match {}".format(pre))
                leader.append(
                    ParseMatchPair(
                        ExpressionType.MATCH_PAIR,
                        expr,
                        self.token,
                        mpairname,
                        pre=pre))


class MatchPairs(CollectionAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value
        self._prefix = None

    @property
    def value(self):
        return self._value

    def elements(self):
        return len(self.value)

    @property
    def prefix(self):
        return self._prefix

    @prefix.setter
    def prefix(self, prefix):
        self._prefix = prefix

    def eval(self, bundle, leader):
        for c in self.value:
            c.eval(bundle, leader)


class Match(FoidlAst):
    def __init__(self, mres, mpred, mexprres, value, token, src):
        super().__init__(token, src)
        self._predicate = mpred
        self._value = value
        # Identify symbol if exists
        sp = token.getsourcepos()
        if mres:
            self._result = mres
        else:
            msym = "matchres_" + str(sp.lineno) + "_" + str(sp.colno)
            self._result = Symbol(
                msym,
                Token(
                    "MATCH_RES",
                    msym,
                    token.getsourcepos()), src)
        if mexprres:
            self._exprres = mexprres
        else:
            msym = "matchexpr_" + str(sp.lineno) + "_" + str(sp.colno)
            self._exprres = Symbol(
                msym,
                Token(
                    "MATCH_EXPR_RES",
                    msym,
                    token.getsourcepos()), src)
        self._ident = "match_" + str(sp.lineno) + "_" + str(sp.colno)

    @property
    def result(self):
        return self._result

    @property
    def exprres(self):
        return self._exprres

    @property
    def predicate(self):
        return self._predicate

    @property
    def value(self):
        return self._value

    @property
    def ident(self):
        return self._ident

    def eval(self, bundle, leader):
        # Create result reference
        mres = MatchResReference(self.result)
        mres.ident = "matchres_" + str(self.token.getsourcepos().lineno) \
            + "_" + str(self.token.getsourcepos().colno)

        mexpr = MatchExprReference(self.exprres)
        mexpr.ident = "matchexpr_" + str(self.token.getsourcepos().lineno) \
            + "_" + str(self.token.getsourcepos().colno)

        # Fetch the predicate statement
        pred = []
        self.predicate.eval(bundle, pred)

        # Expose the predicate reference to lexical scope for expressions
        bundle.symtree.push_scope(self.ident, self.ident)
        bundle.symtree.register_symbol(self.exprres.value, mexpr)
        exprs = []
        self.value[0].eval(bundle, exprs)
        bundle.symtree.pop_scope()

        havedef = None
        for x in range(len(exprs)):
            if type(exprs[x]) is ParseMatchDefault:
                havedef = exprs.pop(x)
                exprs.append(havedef)
                break

        if not havedef:
            mp = MatchPair(
                [None, _NIL],
                self.value[0].token,
                self.value[0].source,
                True)
            mp.eval(bundle, exprs)
        # for e in self.value:
        #     e.eval(bundle, leader)
        # Register result in symbol table
        leader.append(
            ParseMatch(
                ExpressionType.MATCH,
                exprs,
                self.token,
                self.ident,
                [mres, mexpr],
                pred))
        print("Leader {}".format(leader))
        bundle.symtree.register_symbol(self.result.value, mres)


class Lambda(FoidlAst):
    def __init__(self, arguments, value, token, src):
        super().__init__(token, src)
        self.arguments = arguments
        self.value = value
        sp = token.getsourcepos()
        lsym = "lambda_" + str(sp.lineno) + "_" + str(sp.colno)
        self.name = Symbol(lsym, token, src)

    def eval(self, bundle, leader):
        bundle.symtree.push_scope(self.name, self.name)
        i = 0
        argref = []
        for a in self.arguments.value:
            ar = FuncArgReference(a, a.name, i)
            bundle.symtree.register_symbol(a.name, ar)
            argref.append(ar)
            i += 1
        # Eval expression
        expr = []
        for e in self.value:
            e.eval(bundle, expr)
        # Append Body
        plambda, plamdaref = refactor_lambda(
            self, self.token.getsourcepos(), argref, expr,
            [FuncArgReference, LetArgReference,
                LetResReference, MatchResReference])
        # [print("Lamb Arg {}".format(n)) for n in plambda.pre]
        bundle.lambdas.append(plambda)
        # Pop stack
        bundle.symtree.pop_scope()
        # Append Reference
        leader.append(plamdaref)


class Partial(FoidlAst):
    _invalid_types = [ParseLiteral]

    def __init__(self, value, token, src):
        super().__init__(token, src)
        self.value = value
        sp = token.getsourcepos()
        self.name = "partial_" + str(sp.lineno) + "_" + str(sp.colno)

    def _partial_ok(self, parray):
        if type(parray[0]) is not ParseSymbol:
            raise errors.SymbolException(
                "Partial does not expect type {} in first position".format(
                    parray[0]))
        p0expr = parray[0].exprs[0]
        if type(p0expr) is VarReference:
            check = p0expr.exprs[0]
            if type(check) is ParseSymbol:
                check = check.exprs[0]
        else:
            check = p0expr

        if type(check) in self._invalid_types:
            raise errors.SymbolException(
                "Partial not well formed with literal first arg")
        return check

    def _eval_call(self, name, args, bundle, leader):
        """Optimized partial to functionc call"""
        leader.append(
            ParseCall(
                ExpressionType.FUNCTION_CALL,
                args,
                self.token,
                name))

    def _eval_decl(self, ktype, args, bundle, leader):
        """Partial declaration"""
        leader.append(
            ParsePartialDecl(
                ExpressionType.PARTIAL_DECL,
                args,
                self.token,
                self.name,
                pre=ktype))

    def _eval_invk(self, ktype, args, bundle, leader):
        """Partial invocation"""
        leader.append(
            ParsePartialInvk(
                ExpressionType.PARTIAL_INVK,
                args,
                self.token,
                self.name,
                pre=ktype))

    @methdispatch
    def _eval_partial(self, ktype, array, bundle, leader):
        print("_eval_partial unhandled {}".format(ktype))

    @_eval_partial.register(FuncReference)
    def _epfr(self, ktype, array, bundle, leader):
        """Evaluate function reference and optimize to call"""
        argcnt = len(array)
        if ktype.argcnt == argcnt:
            self._eval_call(ktype.name, array, bundle, leader)
        else:
            self._eval_decl(ktype, array, bundle, leader)

    @_eval_partial.register(ParseLambdaRef)
    def _eplr(self, ktype, array, bundle, leader):
        """Evaluate lambda reference and optimize to call"""
        argcnt = len(array)
        if len(ktype.exprs) == argcnt:
            self._eval_call(ktype.name, array, bundle, leader)
        else:
            self._eval_decl(ktype, array, bundle, leader)

    @_eval_partial.register(LetArgReference)
    @_eval_partial.register(FuncArgReference)
    def _epfar(self, ktype, array, bundle, leader):
        """Assume a partial invocation"""
        self._eval_invk(ktype, array, bundle, leader)

    def eval(self, bundle, leader):
        exprs = []
        for e in self.value:
            e.eval(bundle, exprs)
        first = self._partial_ok(exprs)
        self._eval_partial(first, exprs[1:], bundle, leader)


class If(FoidlAst):
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._ident = "if_" + str(token.getsourcepos().lineno) \
            + "_" + str(token.getsourcepos().colno)
        self.value = value
        self._pred, self._then, self._else = value

    @property
    def ident(self):
        return self._ident

    @classmethod
    def generate(cls, value, token, src):
        if len(value) > 3:
            rem = value[3:]
            el = If(value[0:3], token, src)
            rem.insert(0, el)
            return rem
        elif len(value) < 3:
            raise errors.SyntaxError(
                "If expects 3 expressions: pred then else ")
        else:
            return If(value, token, src)

    def eval(self, bundle, leader):
        pred = []
        self._pred.eval(bundle, pred)
        exprs = []
        self._then.eval(bundle, exprs)
        self._else.eval(bundle, exprs)

        leader.append(
            ParseIf(
                ExpressionType.IF,
                exprs,
                self.token,
                self.ident,
                self.ident + "_res",
                pred))


class FunctionCall(FoidlAst):
    def __init__(self, csite, value, token, src):
        super().__init__(token, src)
        if value and type(value[0]) is Expressions:
            self.value = value[0].value
        else:
            self.value = value
        self.call_site = csite

    @classmethod
    def generate(cls, csite, value, token, src, state):
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

    def eval(self, bundle, leader):
        args = []
        for e in self.value:
            e.eval(bundle, args)
        sym = bundle.symtree.resolve_symbol(self.call_site)
        if sym and sym.source != self.source:
            if not bundle.externs.get(self.call_site, None):
                bundle.externs[self.call_site] = sym
        leader.append(
            ParseCall(
                ExpressionType.FUNCTION_CALL,
                args,
                self.token,
                self.call_site))


_NIL = Symbol('nil', Token("SYMBOL", "nil"), None)
