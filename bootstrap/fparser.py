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
import traceback
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


def _panic(cause, token):
    raise errors.ParseError(
        cause.format(
            token.getsourcepos().lineno,
            token.getsourcepos().colno,
            token.gettokentype()))


def _malformed(token):
    print("Token = {}".format(token))
    traceback.print_stack()
    _panic("{}:{} Badly formed statement for {}", token)


def _unexpected(token):
    _panic("{}:{} Unexpected expression for {}", token)


def _unexpected_declaration(token, decltype):
    raise errors.ParseError(
        "{}:{} Unexpected type {} found in expression {}".format(
            token.getsourcepos().lineno,
            token.getsourcepos().colno,
            decltype,
            token.gettokentype()))


def _check_no_declarations(token, values, decltypes):
    [_unexpected_declaration(token, x) for x in values
        if isinstance(x, decltypes)]


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
            _malformed(self.nafter)
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
                _malformed(self.nafter)
        elif self.nafter.gettokentype() == 'SYMBOL':
            symbol = self.nafter
        else:
            _malformed(self.nafter)

        vhdr = self._vfhdrs.get(symbol.getstr(), None)
        if vhdr:
            print("Redefinition of var {}".format(symbol.getstr()))
            _malformed(self.nafter)

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
        elif self.nafter.gettokentype() in ['SYMBOL', 'SYMBOL_PRED', 'SYMBOL_BANG']:
            symbol = self.nafter
        else:
            _malformed(self.nafter)

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
            _malformed(x)

        hit, stuff = self.tokens.get_until(self._ttype_tuple, True)
        fhdr = self._vfhdrs.get(symbol.getstr(), None)
        if fhdr:
            print("Redefinition of func {}".format(symbol.getstr()))
            _malformed(self.nafter)

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
        x = "{}".format(frame)
        _panic("{}:{} Not being handled {} " + x, token)

    @_parse.register(MODULE)
    def parse_module(self, token, frame):
        symbol = None
        # Edge - var token only at end
        if not frame:
            _malformed(token)
        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
            return [ast.Module(symbol, frame, token, self.input)]
        else:
            _panic("{}:{} Expected symbol for {}", token)

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
            _malformed(token)

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
            _panic("{}:{} Missing symbol for {}", token)

        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
        # Should never get here due to pre-parse
        else:
            _panic("{}:{} Expected symbol for {}", token)

        # Check for expression existance
        if frame:
            if not isinstance(frame[0], self._decl_set):
                # TODO: Get until var, func, include or end
                expression = [frame.pop(0)]

        hdr = self._locals.get(symbol.name, None)
        if not hdr:
            _panic("{}:{} Variable {} not in symbol table", symbol.token)
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
            _malformed(token)
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
            _panic("{}:{} Missing symbol for {}", token)

        if isinstance(frame[0], ast.Symbol):
            symbol = frame.pop(0)
        else:
            _panic("{}:{} Expected symbol for {}", token)

        # Should never get here due to pre-parse, but
        if not frame:
            _panic("{}:{} Missing function arguments", token)
        elif not isinstance(frame[0], ast.CollectionAst):
            _panic("{}:{} Expected function arguments", token)
        else:
            arguments = frame.pop(0)

        # Verify all in collection are symbols
        if not isinstance(arguments, ast.EmptyCollection):
            for x in arguments.value:
                if ((not isinstance(x, ast.Symbol) or
                        x.token.gettokentype() not in self._strict_symtokens)):
                    _panic(
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
            _panic("{}:{} Function {} not in symbol table", symbol.token)

        frame.insert(
            0,
            ast.Function(hdr, expressions, self.input))
        return frame

    def _process_call(self, token, frame):
        """Construct the function call"""
        # Maybe able to take advantage of the function
        # signatures here
        fcall = ast.FunctionCall.generate(
            token.token.getstr(),
            frame, token.token, self.input, self.state)
        if isinstance(fcall, ast.FoidlAst):
            fcall = [fcall]
        else:
            pass
        # _check_no_declarations(token, fcall, self._decl_set)
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
        """Parse a function call"""
        return self._process_call(token, frame)

    @_parse.register(LAMBDA)
    def parse_lambda(self, token, frame):
        """Parse Lambda expression"""
        if len(frame) < 2:
            _malformed(token)
        if not isinstance(frame[0], ast.CollectionAst):
            _panic("{}:{} Lambda expect argument signature {}", token)
        elif frame[0].ctype is not CollTypes.LIST:
            _panic("{}:{} Lambda expect argument list {}", token)

        arguments = frame.pop(0)
        expression = [frame.pop(0)]
        _check_no_declarations(token, expression, self._decl_set)
        frame.insert(
            0,
            ast.Lambda(arguments, expression, token, self.input))
        return frame

    @_parse.register(MATCH)
    def parse_match(self, token, frame):
        """Parse Match expression"""
        matchres = None
        matchexpr = None
        matchpairs = None
        # Must have length expected
        if len(frame) < 2:
            _malformed(token)
        index = next(
            (i for i, item in enumerate(frame)
                if isinstance(item, ast.MatchPairs)), -1)
        # Must have a MatchPairs for semantic correctness
        if index < 0:
            _panic(
                "{}:{} Match requires at least one guard pair '|' {}",
                token)
        # Must have at least an expression prior to MatchPairs
        elif index == 0:
            _panic(
                "{}:{} Match requires expression before guard pairs {}",
                token)
        # match expression pairs
        elif index == 1:
            matchexpr = frame.pop(0)
            matchpairs = frame.pop(0)
        # match result expression pairs
        elif index == 2:
            if isinstance(frame[0], ast.Symbol):
                matchres = frame.pop(0)
            else:
                _panic("{}:{} Match result must be a symbol", token)
            matchexpr = frame.pop(0)
            matchpairs = frame.pop(0)
        else:
            _malformed(token)
        frame.insert(
            0,
            ast.Match(matchres, matchexpr, matchpairs, token, self.input))
        return frame

    @_parse.register(MATCH_GUARD)
    def parse_match_guard(self, token, frame):
        if len(frame) < 2:
            _malformed(token)
        value = frame[0:2]
        _check_no_declarations(token, value, self._decl_set)
        frame = frame[2:]
        default = False
        if (isinstance(value[0], ast.LiteralReference) and
                value[0].value == ':default'):
                default = True
        p = ast.MatchPair(
            value,
            token,
            self.input,
            default)
        if frame and isinstance(frame[0], ast.MatchPairs):
            if default:
                [_panic(
                    "{}:{} Only one :default expression in match allowed",
                    token)
                    for x in frame[0].value if x.default]
            frame[0].value.append(p)
        else:
            frame.insert(
                0,
                ast.MatchPairs([p], token, self.input))
        return frame

    @_parse.register(MATCH_EXPRREF)
    def parse_match_expression_ref(self, token, frame):
        """Parse occurance of '%0'"""
        frame.insert(
            0,
            ast.MatchExpressionRef(token.getstr(), token, self.input))
        return frame

    @_parse.register(LET)
    def parse_let(self, token, frame):
        """Parse Let expression"""
        if len(frame) < 2:
            _malformed(token)
        # Resolve first as return val versus args
        letres = None
        letargs = None
        letexpr = None

        if isinstance(frame[0], ast.Symbol):
            letres = frame.pop(0)
        else:
            sp = token.getsourcepos()
            lsym = "let_" + str(sp.lineno) + "_" + str(sp.colno)
            letres = ast.Symbol(
                lsym,
                Token(
                    "LET_RES",
                    lsym,
                    token.getsourcepos()), self.input)

        if isinstance(frame[0], ast.CollectionAst):
            letargs = frame.pop(0)
        else:
            _panic(
                "{}:{} Let expects local argument pair(s) signature [...] {}",
                token)

        if letargs.ctype != CollTypes.LIST:
            _panic(
                "{}:{} Invalid let argument pair(s) type. Should be [...] {}",
                token)

        if len(frame) < 1:
            _panic("{}:{} Missing Let expression {}", token)

        letexpr = [frame.pop(0)]
        letargs = ast.LetPairs(letargs.value)
        _check_no_declarations(token, letexpr, self._decl_set)
        frame.insert(
            0,
            ast.Let(letres, letargs, letexpr, token, self.input))
        return frame

    @_parse.register(IF)
    def parse_if(self, token, frame):
        if len(frame) < 3:
            _malformed(token)
        value = frame[0:3]
        _check_no_declarations(token, value, self._decl_set)
        frame = frame[3:]
        frame.insert(
            0,
            ast.If(value, token, self.input))
        return frame

    def group_partial_handdler(self, token, frame, clz):
        """Handle either group or partial"""
        index = next(
            (i for i, item in enumerate(frame)
                if isinstance(item, RPAREN)), -1)
        if index < 0:
            _panic("{}:{} Expression requires terminating ')' {}", token)
        # If rparen is index 0, it is empty expression
        elif index == 0:
            frame.pop(0)
        else:
            value = frame[0:index]
            frame = frame[index + 1:]
            # Can not contain declaration
            _check_no_declarations(token, value, self._decl_set)
            # If there is only 1 value and it is same type
            # insert the value back into frame, optimizing
            # away the outer type
            if len(value) == 1 and isinstance(value[0], clz):
                frame.insert(0, value[0])
            else:
                frame.insert(
                    0,
                    clz(value, token, self.input))
        return frame

    @_parse.register(GROUP)
    def parse_group(self, token, frame):
        """Parse Group expression"""
        return self.group_partial_handdler(token, frame, ast.Group)

    @_parse.register(LPAREN)
    def parse_partial(self, token, frame):
        """Prase LPAREN which indicates a partial expression"""
        return self.group_partial_handdler(token, frame, ast.Partial)

    @_parse.register(RPAREN)
    def parse_rparen(self, token, frame):
        """Parse partial or group expression terminators"""
        frame.insert(0, token)
        return frame

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
        # No matching closing token
        if index < 0:
            _panic("{}:{} Unexpected {}", token)
        # Empty collection
        elif index == 0:
            frame.pop(0)
            item = ast.EmptyCollection.generate(ctype, token, self.input)
        # Collection with elements
        else:
            value = frame[0:index]
            frame = frame[index + 1:]
            _check_no_declarations(token, value, self._decl_set)
            if isinstance(token, LBRACE) and len(value) % 2 != 0:
                _panic(
                    "{}:{} Map data types requires even number of elements",
                    token)
            item = ast.Collection(ctype, value, token, self.input)
        frame.insert(0, item)
        return frame

    @_parse.register(RANGLE)
    @_parse.register(RBRACKET)
    @_parse.register(RBRACE)
    def parse_collection_stop(self, token, frame):
        """Parse collection type terminators"""
        frame.insert(0, token)
        return frame

    @_parse.register(EQ_REF)
    @_parse.register(LT_REF)
    @_parse.register(GT_REF)
    @_parse.register(LTEQ_REF)
    @_parse.register(GTEQ_REF)
    @_parse.register(NOTEQ_REF)
    # @_parse.register(MATH_REF)
    def parse_common_references(self, token, frame):
        frame.insert(0, ast.Symbol(token.getstr(), token.token, self.input))
        return frame

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
            # for x in ar[0].value:
            #     print(x)
            return ast.CompilationUnit(ar)
        else:
            return self

    def get_parser(self):
        return self
