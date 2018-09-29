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

    @property
    def argpos(self):
        return self._argpos


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


class LetResReference(FoidlReference):
    """Let Result Symbol Reference"""

    def __init__(self, astmember):
        super().__init__(astmember)

    def eval(self, bundle, leader):
        leader.append({
            "type": 'let_result',
            "expr": [self]})


class MatchResReference(FoidlReference):
    """Match Result Symbol Reference"""

    def __init__(self, result_id):
        self._res_id = result_id

    @property
    def id(self):
        return self._res_id


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

    def eval(self, bundle, ptree):
        # Includes already handled
        # Register new level in bundle
        bundle.symtree.push_scope(self.name, self.name)
        # Add references to nil, true, false
        bundle.externs = {
            "nil": bundle.symtree.resolve_symbol("nil"),
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
    def __init__(self, value, token, src):
        super().__init__(token, src)
        self._value = value

    @property
    def value(self):
        return self._value

    def eval(self, bundle, leader):
        pass


class VarHeader(FoidlAst):
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
        self._value = value if value else [_NIL]

    @property
    def name(self):
        return self._name.value

    @property
    def value(self):
        return self._value

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


class MatchPairs(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, bundle, leader):
        for c in self.value:
            c.eval()


class EmptyCollection(CollectionAst):
    def __init__(self, etype, value, token, src):
        super().__init__(token, src)
        self.type = etype
        self.value = value

    def elements(self):
        return 0

    @classmethod
    def get_empty_collection(cls, ct, t, src):
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

    def elements(self):
        return len(self.value)

    def eval(self, bundle, leader):
        LOGGER.info("Collection {} value {}".format(self.type, self.value))
        members = []
        for c in self.value:
            c.eval(bundle, members)

        leader.append(
            ParseTree(
                ExpressionType.COLLECTION,
                members,
                self.token,
                res=self.type))


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
    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        print("Group value {}".format(self.value))
        for e in self.value:
            e.eval(bundle, leader)


class LetPairs(CollectionAst):
    def __init__(self, value):
        self.value = value

    def elements(self):
        return len(self.value)

    def eval(self, bundle, leader):
        # Put symbols in table and evaluate expression
        for i, j in zip(*[iter(self.value)] * 2):
            expr = []
            j.eval(bundle, expr)
            lar = LetArgReference(i)
            bundle.symtree.register_symbol(i.value, lar)
            leader.append(
                ParseTree(
                    ExpressionType.LET_ARG_PAIR,
                    expr,
                    name=i.value))


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

    @classmethod
    def re_factor(cls, p, src):
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
            letpairs = EmptyCollection.get_empty_collection(
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
        bundle.symtree.push_scope(self._id, self._id)
        # Push Let scope on bundle
        # Register any letpair vars in table
        preblock = []
        self._letpairs.eval(bundle, preblock)
        # Process
        expr = []
        for e in self.value:
            e.eval(bundle, expr)
        # Pop scope
        bundle.symtree.pop_scope()
        lres = LetResReference(self._id)
        bundle.symtree.register_symbol(self._id.value, lres)
        # Register id
        # Set let in leader
        leader.append(
            ParseLet(
                ExpressionType.LET,
                expr,
                self.token,
                self._id.value,
                ParseTree(ExpressionType.LET_RES, lres),
                preblock))


class Match(FoidlAst):
    def __init__(self, value):
        self.value = value

    def eval(self, bundle, leader):
        print("Match value {}".format(self.value))
        for e in self.value:
            e.eval(bundle, leader)


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
        bundle.lambdas.append(
            ParseLambda(
                ExpressionType.LAMBDA,
                expr,
                self.token,
                self.name.value,
                pre=argref))
        # Pop stack
        bundle.symtree.pop_scope()
        leader.append(
            ParseLambdaRef(
                ExpressionType.LAMBDA_REF,
                argref,
                self.token,
                self.name.value))
        # leader.append(LambdaReference(
        #     self,
        #     self.name.value,
        #     len(self.arguments.value)))


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
        self.value = value

    def eval(self, bundle, leader):
        print("If value {}".format(self.value))
        for e in self.value:
            e.eval(bundle, leader)


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


_NIL = Symbol('nil', Token("SYMBOL", "nil"), None)
