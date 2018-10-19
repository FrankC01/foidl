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
import pprint
from functools import singledispatch, update_wrapper

from rply import Token

from enums import CollTypes
import ast
import errors
from handler import include_parse
from foidl_token import *


LOGGER = logging.getLogger()


def methdispatch(func):
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


def _collection_type(p0):
    x = None
    if p0.gettokentype() is 'LSET':
        x = CollTypes.SET
    elif p0.gettokentype() is 'LBRACKET':
        x = CollTypes.LIST
    elif p0.gettokentype() is 'LBRACE':
        x = CollTypes.MAP
    else:
        x = CollTypes.VECTOR
    return x


def _token_eater(in_list, base_type, out_list=None):
    if not out_list:
        out_list = []
    for n in in_list:
        if type(n) is not Token:
            if type(n) is base_type:
                _token_eater(n.value, base_type, out_list)
            else:
                out_list.append(n)
        else:
            pass
    return base_type(out_list)


def _flatten_list(in_list, out_list=None):
    if not out_list:
        out_list = []
    for n in in_list:
        if type(n) is list:
            out_list = _flatten_list(n, out_list)
        else:
            out_list.append(n)
    return out_list


def _literal_entry(literals, token, srcstr):
    ttype = token.gettokentype()
    tval = token.getstr()
    littype = literals[ttype]
    litref = littype.get(tval, None)
    if not litref:
        litref = ast.LiteralReference(
            str(len(littype)),
            token, srcstr)
        littype[tval] = litref
    return litref


def math_func(token):
    s = token.getstr()
    if s == "+":
        return "add"
    elif s == "*":
        return "mul"
    elif s == "/":
        return "div"
    elif s == "-":
        return "sub"
    else:
        raise errors.ParseError(
            "{} not recognized as math operator".format(s))


class DONT_USE_OBSOLETE():
    def __init__(self, mlexer, input):
        # A list of all token names accepted by the parser.
        # self.pg = ParserGenerator(
        #     mlexer.get_tokens(),
        #     # precedence=[])
        #     precedence=[])
        self._input = input

    @property
    def input(self):
        return self._input

    def parse(self):

        # @self.pg.production('expression : math_call')
        @self.pg.production('expression : logic_call')
        @self.pg.production('expression : partial')
        @self.pg.production('expression : group')
        @self.pg.production('expression : lambda')
        @self.pg.production('expression : if')
        @self.pg.production('expression : let')
        @self.pg.production('expression : match')
        def expression(state, p):
            # print("single expression = {} {}".format(p, p[0]))
            return p[0]

        @self.pg.production('call_signature : MATH_CALL expression expression')
        @self.pg.production('call_signature : FUNC_CALL expressions')
        @self.pg.production('call_signature : FUNC_BANG expressions')
        @self.pg.production('call_signature : FUNC_PRED expressions')
        def call_signature(state, p):
            print("CS {}".format(p))
            return p

        @self.pg.production('logic_call : LT_CALL expression expression')
        @self.pg.production('logic_call : GT_CALL expression expression')
        @self.pg.production('logic_call : LTEQ_CALL expression expression')
        @self.pg.production('logic_call : GTEQ_CALL expression expression')
        @self.pg.production('logic_call : NOTEQ_CALL expression expression')
        @self.pg.production('logic_call : EQ_CALL expression expression')
        def logic_call(state, p):
            p = p[0]
            t = p.pop(0)
            fcall = ast.FunctionCall.generate(
                t.getstr(), p, t, self.input, state)
            # print("FC returning {}".format(fcall))
            return fcall

        @self.pg.production('partial : LPAREN expressions RPAREN')
        def partial(state, p):
            """Partial parse"""
            p.pop(2)
            t = p.pop(0)
            return ast.Partial(p[0].value, t, self.input)

        @self.pg.production('group : GROUP RPAREN')
        @self.pg.production('group : GROUP expressions RPAREN')
        def group(state, p):
            """Group parse for zero or more expressions"""
            if len(p) < 3:
                return ast.Group([], p[0], self.input)
            else:
                del p[2]
                t = p.pop(0)
                return ast.Group(p, t, self.input)

        @self.pg.production('lambda : LAMBDA symbol_list expression')
        def lambdaexpr(state, p):
            """Lambda parse"""
            t = p.pop(0)
            return ast.Lambda(p.pop(0), p, t, self.input)

        @self.pg.production('if : IF expression expression expression')
        def ifexpr(state, p):
            """If parse"""
            i = p.pop(0)
            return ast.If.generate(_flatten_list(p), i, self.input)

        @self.pg.production('let : LET let_locals expression')
        @self.pg.production(
            'let : LET strict_symbol let_locals expression')
        def letexpr(state, p):
            """Let parse"""
            return ast.Let.generate(_flatten_list(p), self.input)

        @self.pg.production("let_locals : LBRACKET RBRACKET")
        @self.pg.production("let_locals : LBRACKET letpairs RBRACKET")
        def let_locals(state, p):
            return p

        @self.pg.production('letpairs : strict_symbol expression')
        @self.pg.production('letpairs : strict_symbol expression letpairs')
        def letpairs(state, p):
            """Let parse support for zero or more local var assignments"""
            return _token_eater(_flatten_list(p), ast.LetPairs)

        @self.pg.production('match : MATCH expression matchpairs')
        @self.pg.production(
            'match : MATCH strict_symbol expression matchpairs')
        def match(state, p):
            """Match parse"""
            # print("Match expr {}".format(p))
            t = p.pop(0)
            mres = None
            if len(p) == 3:
                mres = p.pop(0)
            mpred = p.pop(0)
            return ast.Match(
                mres, mpred,
                [x for x in p if type(x) is not Token], t, self.input)

        @self.pg.production(
            'matchpairs : MATCH_PATTERN expression expression')
        @self.pg.production(
            'matchpairs : MATCH_PATTERN expression expression matchpairs')
        @self.pg.production(
            'matchpairs : MATCH_PATTERN MATCH_DEFAULT expression')
        def matchpairs(state, p):
            # print("Match Pair {}".format(p))
            t = p.pop(0)

            def eater(in_list):
                out_list = []
                for n in in_list:
                    if type(n) is ast.MatchPair:
                        out_list.append(n)
                    elif type(n) is ast.MatchPairs:
                        out_list.extend(eater(n.value))
                    else:
                        print("MATCH PAIRS UNKNOWN {}".format(n))
                return out_list

            def isdef(el):
                return True \
                    if type(el) is Token and \
                    el.getstr() == ':default' else False

            res = []
            if len(p) == 2:
                res.append(ast.MatchPair(p, t, self.input, isdef(p[0])))
            elif len(p) > 2:
                pattern = p.pop(0)
                expr = p.pop(0)
                elem = ast.MatchPair(
                    [pattern, expr], t, self.input, isdef(pattern))
                p.insert(0, elem)
                res = p

            y = eater(res)
            return ast.MatchPairs(y, t, self.input)

        @self.pg.production("symbol_list : empty_list")
        @self.pg.production("symbol_list : LBRACKET strict_symbols RBRACKET")
        def symbol_list(state, p):
            if len(p) < 3:
                return ast.EmptyCollection(
                    CollTypes.LIST,
                    [],
                    p[0],
                    self.input)
            else:
                return p[1]

        @self.pg.production("symbol_only_list : empty_list")
        @self.pg.production(
            "symbol_only_list : LBRACKET symbols_only RBRACKET")
        def symbol_only_list(state, p):
            if len(p) < 3:
                return ast.EmptyCollection(
                    CollTypes.LIST,
                    [],
                    p[0],
                    self.input)
            else:
                return p[1]

        @self.pg.production('symbol : MATCH_EXPRREF')
        def matchexprref(state, p):
            return ast.MatchExpressionRef(p[0].getstr(), p[0], self.input)

        @self.pg.production('symbol : MATH_REF')
        def symbol_mathref(state, p):
            return ast.Symbol(math_func(p[0]), p[0], self.input)

        @self.pg.production('symbol : IF_REF')
        @self.pg.production('symbol : EQ_REF')
        @self.pg.production('symbol : LT_REF')
        @self.pg.production('symbol : GT_REF',)
        @self.pg.production('symbol : LTEQ_REF')
        @self.pg.production('symbol : GTEQ_REF')
        @self.pg.production('symbol : NOTEQ_REF')
        def symbol_logref(state, p):
            # This needs a lookup resolver
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production('symbol : SYMBOL')
        def symbol(state, p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

    def get_parser(self):
        return self.pg.build()


class _TDParser(object):

    _ttype_tuple = (INCLUDE, VAR, FUNC)

    def __init__(self):
        super().__init__()
        self._vfhdrs = {}
        self._result = []
        self._include = []
        self._rinclude = []
        self._nafter = None

    @property
    def state(self):
        return self._state

    @property
    def input(self):
        return self._input

    @property
    def tokens(self):
        return self._tokens

    @property
    def nafter(self):
        return self._nafter

    @nafter.setter
    def nafter(self, nafter):
        self._nafter = nafter

    @property
    def result(self):
        return self._result

    def include(self):
        symlist = []
        if self.nafter.gettokentype() == "LBRACKET":
            hit, stuff = self.tokens.get_until(self._ttype_tuple, True)
            self._rinclude.extend(stuff)
            symlist = stuff[0:-1]
        elif self.nafter.gettokentype() == "SYMBOL":
            symlist.append(self.nafter)
        else:
            raise IOError
        [self._include.append(
            ast.Symbol(x.getstr(), x, self.input)) for x in symlist
            if x.gettokentype() == 'SYMBOL']

    def variable(self):
        symbol = None
        private = False
        hit, stuff = self.tokens.get_until(self._ttype_tuple, True)
        if self.nafter.gettokentype() == 'KEYWORD':
            if self.nafter.getstr() == ":private":
                private = True
                if stuff and stuff[0].gettokentype() == 'SYMBOL':
                    symbol = stuff[0]
            else:
                raise IOError
        elif self.nafter.gettokentype() == 'SYMBOL':
            symbol = self.nafter
        else:
            raise IOError

        vhdr = self._vfhdrs.get(symbol.getstr(), None)
        if vhdr:
            print("Redefinition of var {}".format(symbol.getstr()))
            raise IOError

        self._vfhdrs[symbol.getstr()] = ast.VarHeader(
            ast.Symbol(symbol.getstr(), symbol, self.input),
            symbol, self.input, private)


    def function(self):
        symbol = None
        private = False
        arguments = None
        if self.nafter.gettokentype() == 'KEYWORD':
            if self.nafter.getstr() == ":private":
                private = True
                x = next(self.tokens)
                if x and x.gettokentype() == 'SYMBOL':
                    symbol = x
                else:
                    raise IOError
        elif self.nafter.gettokentype() == 'SYMBOL':
            symbol = self.nafter
        else:
            raise IOError

        x = next(self.tokens)
        if x and x.gettokentype() == 'LBRACKET':
            hit, stuff = self.tokens.get_until((RBRACKET), True)
            if not hit:
                raise IOError
            next(self.tokens)
            args = [ast.Symbol(x.getstr(), x, self.input) for x in stuff]
            if not args:
                arguments = ast.EmptyCollection(
                    CollTypes.LIST, args, x, self.input)
            else:
                arguments = ast.Collection(
                    CollTypes.LIST, args, x, self.input)
        else:
            raise IOError()

        hit, stuff = self.tokens.get_until(self._ttype_tuple, True)
        fhdr = self._vfhdrs.get(symbol.getstr(), None)
        if fhdr:
            print("Redefinition of func {}".format(symbol.getstr()))
            raise IOError

        self._vfhdrs[symbol.getstr()] = ast.FuncHeader(
            ast.Symbol(symbol.getstr(), symbol, self.input),
            arguments, symbol, self.input, private)

    def body(self):
        while self.tokens.ismore:
            found, fluff = self.tokens.get_until(self._ttype_tuple, True)
            if found:
                # print("Found {}".format(found))
                hit = next(self.tokens)
                self.nafter = next(self.tokens)
                if found.gettokentype() == 'VAR':
                    self.variable()
                elif found.gettokentype() == 'FUNC':
                    self.function()
                else:
                    self._rinclude.extend([hit, self.nafter])
                    self.include()
            else:
                break

    def parse(self, state, tokens, input):
        self._state = state
        self._tokens = tokens
        self._input = input
        self.body()
        if self._include:
            # Consolidate includes
            # Pre-process includes
            include_parse(
                self.state,
                ast.Include(self._include, self._rinclude[0], self.input))
            # Remove includes
            tokens.drop_tokens(self._rinclude)
        # Register functions
        state.symtree.push_scope(self.input, self.input)
        [state.symtree.register_symbol(
            x, y.get_reference())
            for (x, y) in self._vfhdrs.items()]
        return self._vfhdrs


class AParser(object):
    _collection_end = {
        "{": "}",
        "#{": "}",
        "<": ">",
        "[": "]"}

    _strict_symtokens = ['SYMBOL', 'SYMBOL_BANG', 'SYMBOL_PRED']

    _decl_set = (ast.Variable, ast.Function, ast.Include)

    def __init__(self, mlexer, input):
        self._input = input
        self._tokens = None
        self._state = None
        self._locals = None

    def _panic(self, cause, token):
        raise errors.ParseError(
            cause.format(
                token.getsourcepos().lineno,
                token.getsourcepos().colno,
                token.getstr()))

    def _malformed(self, token):
        self._panic("{}:{} Badly formed statement for {}", token)

    def _unexpected(self, token):
        self._panic("{}:{} Unexpected expression for {}", token)

    @property
    def input(self):
        return self._input

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, state):
        self._state = state

    @property
    def tokens(self):
        return self._tokens

    @tokens.setter
    def tokens(self, tokens):
        self._tokens = tokens

    @methdispatch
    def _parse(self, token, frame):
        print("{} not handled yet".format(token))
        frame.insert(0, token)
        return frame

    @_parse.register(MODULE)
    def parse_module(self, token, frame):
        symbol = None
        # Edge - var token only at end
        if not frame:
            self._malformed(token)
        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
            return [ast.Module(symbol, frame, token, self.input)]
        else:
            self._panic("{}:{} Expected symbol for {}", token)

    @_parse.register(VAR)
    def parse_variable(self, token, frame):
        """Variable parse. Productions:

        var symbol
        var symbol expression (not = FUNC, VAR, INCLUDE, MODULE)
        var :private symbol
        var :private symbol expression (not = FUNC, VAR, INCLUDE, MODULE)

        if there is an expression, there must be only one
        """
        symbol = None
        expression = None
        # Edge - var token only at end
        if not frame:
            self._malformed(token)

        # Check first for private
        if isinstance(frame[0], ast.LiteralReference):
            if frame[0].value == ":private":
                frame.pop(0)
            # Should never get here due to pre-parse, but
            else:
                raise errors.ParseError(
                    "{}:{} Variable type only supports :private keyword."
                    " Found {}".format(
                        token.getsourcepos().lineno,
                        token.getsourcepos().colno,
                        frame[0].value))

        # Should never get here due to pre-parse
        if not frame:
            self._panic("{}:{} Missing symbol for {}", token)

        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
        # Should never get here due to pre-parse
        else:
            self._panic("{}:{} Expected symbol for {}", token)

        # Check for expression existance
        if frame:
            if not isinstance(frame[0], self._decl_set):
                # TODO: Get until var, func, include or end
                expression = [frame.pop(0)]

        hdr = self._locals.get(symbol.name, None)
        if not hdr:
            self._panic("{}:{} Variable {} not in symbol table", symbol.token)
        frame.insert(
            0,
            ast.Variable(
                hdr,
                expression,
                token, self.input))
        return frame

    @_parse.register(FUNC)
    def parse_function(self, token, frame):
        symbol = None
        arguments = None

        # Edge - func token only at end
        if not frame:
            self._malformed(token)
        # Check first for private
        if isinstance(frame[0], ast.LiteralReference):
            if frame[0].value == ":private":
                frame.pop(0)
            # Should never get here due to pre-parse, but
            else:
                raise errors.ParseError(
                    "{}:{} Function type only supports :private keyword."
                    " Found {}".format(
                        token.getsourcepos().lineno,
                        token.getsourcepos().colno,
                        frame[0].value))
        # Should never get here due to pre-parse, but
        if not frame:
            self._panic("{}:{} Missing symbol for {}", token)

        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
        else:
            self._panic("{}:{} Expected symbol for {}", token)

        # Should never get here due to pre-parse, but
        if not frame:
            self._panic("{}:{} Missing function arguments", token)
        elif not isinstance(frame[0], ast.CollectionAst):
            self._panic("{}:{} Expected function arguments", token)
        else:
            arguments = frame.pop(0)

        # Verify all in collection are symbols
        if not isinstance(arguments, ast.EmptyCollection):
            for x in arguments.value:
                if ((not isinstance(x, ast.Symbol) or
                        x.token.gettokentype() not in self._strict_symtokens)):
                    self._panic(
                        "{}:{} Invalid symbol in arguments for {}", token)

        expressions = []
        index = 0
        # Get all expressions in function
        for x in frame:
            if not isinstance(x, self._decl_set):
                expressions.append(x)
                index += 1
            else:
                break
        frame = frame[index:]

        hdr = self._locals.get(symbol.name, None)
        if not hdr:
            self._panic("{}:{} Function {} not in symbol table", symbol.token)

        frame.insert(
            0,
            ast.Function(hdr, expressions, self.input))
        return frame

    def _process_call(self, token, frame):
        fcall = ast.FunctionCall.generate(
            token.token.getstr(),
            frame, token.token, self.input, self.state)
        if isinstance(fcall, ast.FoidlAst):
            return [fcall]
        else:
            return fcall

    @_parse.register(FUNC_CALL)
    @_parse.register(FUNC_BANG)
    @_parse.register(FUNC_PRED)
    @_parse.register(EQ_CALL)
    @_parse.register(LT_CALL)
    @_parse.register(GT_CALL)
    @_parse.register(LTEQ_CALL)
    @_parse.register(GTEQ_CALL)
    @_parse.register(NOTEQ_CALL)
    @_parse.register(MATH_CALL)
    def parse_call(self, token, frame):
        return self._process_call(token, frame)

    @_parse.register(LANGLE)
    @_parse.register(LBRACKET)
    @_parse.register(LBRACE)
    @_parse.register(LSET)
    def parse_collection_start(self, token, frame):
        """Parse collection types"""
        rhs = self._collection_end[token.getstr()]
        ctype = _collection_type(token)
        index = next(
            (i for i, item in enumerate(frame)
                if hasattr(item, 'getstr')
                if item.getstr() == rhs), -1)
        if index < 0:
            # This is an exception
            raise IOError
        elif index == 0:
            frame.pop(0)
            item = ast.EmptyCollection.generate(ctype, token, self.input)
        else:
            value = frame[0:index]
            frame = frame[index + 1:]
            item = ast.Collection(ctype, value, token, self.input)
        frame.insert(0, item)
        return frame

    @_parse.register(RANGLE)
    @_parse.register(RBRACKET)
    @_parse.register(RBRACE)
    def parse_collection_stop(self, token, frame):
        frame.insert(0, token)
        return frame

    @_parse.register(IF_REF)
    @_parse.register(EQ_REF)
    @_parse.register(LT_REF)
    @_parse.register(GT_REF)
    @_parse.register(LTEQ_REF)
    @_parse.register(GTEQ_REF)
    @_parse.register(NOTEQ_REF)
    @_parse.register(SYMBOL)
    @_parse.register(SYMBOL_BANG)
    @_parse.register(SYMBOL_PRED)
    def parse_symbol(self, token, frame):
        frame.insert(0, ast.Symbol(token.getstr(), token.token, self.input))
        return frame

    @_parse.register(STRING)
    @_parse.register(CHAR)
    @_parse.register(BIT)
    @_parse.register(HEX)
    @_parse.register(REAL)
    @_parse.register(KEYWORD)
    @_parse.register(INTEGER)
    def parse_literal(self, token, frame):
        entry = _literal_entry(self.state.literals, token.token, self.input)
        frame.insert(0, entry)
        return frame

    @_parse.register(COMMA)
    def parse_ignore(self, token, frame):
        return frame

    def parse(self, tokens=None, state=None):
        if tokens:
            self.tokens = FoidlTokenStream(tokens)
            self.state = state
            # Get includes, vars and funcs
            lp = _TDParser()
            self._locals = lp.parse(self.state, self.tokens, self.input)
            self.tokens.set_bottom_up()
            ar = []
            while self.tokens.ismore:
                ar = self._parse(next(self.tokens), ar)
                if not ar:
                    break
            # for x in ar[0].value:
            #     print(x)
            return ast.CompilationUnit(ar)
        else:
            return self

    def get_parser(self):
        return self
