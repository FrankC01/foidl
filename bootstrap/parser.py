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

from rply import ParserGenerator, Token

from enums import CollTypes
import ast
import errors
from handler import include_parse


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


def collection_type(p0):
    return _collection_type(p0)


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


def token_eater(in_list, base_type, out_list=None):
    return _token_eater(in_list, base_type, out_list=None)


def _symbol_only(in_list, out_list=None):
    if not out_list:
        out_list = []
    for n in in_list:
        if type(n) is ast.Symbol:
            out_list.append(n)
        else:
            out_list = _symbol_only(n.value, out_list)
    return out_list


def symbol_only(in_list, out_list=None):
    return _symbol_only(in_list, out_list)


def _flatten_list(in_list, out_list=None):
    if not out_list:
        out_list = []
    for n in in_list:
        if type(n) is list:
            out_list = _flatten_list(n, out_list)
        else:
            out_list.append(n)
    return out_list


def flatten_list(in_list, out_list=None):
    return _flatten_list(in_list, out_list=None)


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


class Parser():
    def __init__(self, mlexer, input):
        # A list of all token names accepted by the parser.
        self.pg = ParserGenerator(
            mlexer.get_tokens(),
            # precedence=[])
            precedence=[
                ("left", ["SYMBOL", "KEYWORD", "AS"]),
                ("left", ["FUNC_CALL", "FUNC_BANG", "FUNC_PRED", "IF"]),
                ("left", ["FUNC", "VAR"]),
                ("left", ["SYMBOL_BANG", "SYMBOL_PRED"])])
        self._input = input

    @property
    def input(self):
        return self._input

    def parse(self):
        @self.pg.production('program : module')
        def program(state, p):
            return ast.CompilationUnit(p)

        @self.pg.production('module : MODULE symbol_only')
        @self.pg.production('module : MODULE symbol_only include_declaration')
        @self.pg.production(
            'module : MODULE symbol_only include_declaration declarations')
        @self.pg.production('module : MODULE symbol_only declarations')
        def module(state, p):
            pp = _flatten_list(p)
            return ast.Module(pp[1], pp[2:], pp[0], self.input)

        @self.pg.production('include_declaration : INCLUDE symbol_only')
        @self.pg.production(
            'include_declaration : INCLUDE symbol_only include_declaration')
        @self.pg.production('include_declaration : INCLUDE symbol_only_list')
        @self.pg.production(
            'include_declaration : INCLUDE symbol_only_list include_declaration')
        def include_declaration(state, p):
            if state.incprocessed:
                raise errors.IncludeOutOfOrder(
                    "Include expression found after declarations")
            t = p.pop(0)
            print("Include {}".format(p))
            i = ast.Include(_symbol_only(p), t, self.input)
            include_parse(state, i)
            return i

        @self.pg.production('declarations : func_declaration')
        @self.pg.production('declarations : var_declaration')
        @self.pg.production('declarations : func_declaration declarations')
        @self.pg.production('declarations : var_declaration declarations')
        def declarations(state, p):
            return _flatten_list(p)

        @self.pg.production('func_declaration : func_header')
        @self.pg.production('func_declaration : func_header expressions')
        def func_declarations(state, p):
            t = p.pop(0)
            return ast.Function(t, p, self.input)

        @self.pg.production('func_header : FUNC strict_symbol symbol_list')
        @self.pg.production(
            'func_header : FUNC KEYWORD strict_symbol symbol_list')
        def func_hdr(state, p):
            t = p.pop(0)
            prv = False
            if type(p[0]) is Token:
                if p[0].gettokentype() == 'KEYWORD' \
                        and p[0].getstr() == ':private':
                        prv = True
                        p.pop(0)
            fh = ast.FuncHeader(p.pop(0), p[0], t, self.input, prv)
            if not state.incprocessed and state.mainsrc == self.input:
                state.incprocessed = True
                state.symtree.push_scope(self.input, self.input)
            state.symtree.register_symbol(fh.name.value, fh.get_reference())
            return fh

        @self.pg.production('var_declaration : var_header')
        @self.pg.production('var_declaration : var_header expression')
        def var_declarations(state, p):
            t = p.pop(0)
            return ast.Variable(t, p, t.token, self.input)

        @self.pg.production('var_header : VAR strict_symbol')
        @self.pg.production('var_header : VAR KEYWORD strict_symbol')
        def var_hdr(state, p):
            t = p.pop(0)
            prv = False
            if type(p[0]) is Token:
                if p[0].gettokentype() == 'KEYWORD' \
                        and p[0].getstr() == ':private':
                        prv = True
                        p.pop(0)
            vh = ast.VarHeader(p.pop(0), t, self.input, prv)
            if not state.incprocessed and state.mainsrc == self.input:
                state.incprocessed = True
                state.symtree.push_scope(self.input, self.input)
            state.symtree.register_symbol(vh.name.value, vh.get_reference())
            return vh

        @self.pg.production('expression : symbol')
        @self.pg.production('expression : literal')
        @self.pg.production('expression : collection')
        @self.pg.production('expression : function_call')
        @self.pg.production('expression : partial')
        @self.pg.production('expression : group')
        @self.pg.production('expression : lambda')
        @self.pg.production('expression : if')
        @self.pg.production('expression : let')
        @self.pg.production('expression : match')
        def expression(state, p):
            # print("single expression = {} {}".format(p, p[0]))
            return p[0]

        @self.pg.production('expressions : expression COMMA expressions')
        @self.pg.production('expressions : expression expressions')
        @self.pg.production('expressions : expression')
        def expressions(state, p):
            """Parse one or more expressions"""
            if len(p) == 1 and type(p[0]) is list:
                p = p[0]
            t = _token_eater(p, ast.Expressions)
            return t

        @self.pg.production('function_call : call_signature')
        def function_call(state, p):
            p = p[0]
            t = p.pop(0)
            fcall = ast.FunctionCall.generate(
                t.getstr(), p, t, self.input, state)
            # print("FC returning {}".format(fcall))
            return fcall

        @self.pg.production('call_signature : MATH_CALL expressions')
        @self.pg.production('call_signature : EQ_CALL expressions')
        @self.pg.production('call_signature : LT_CALL expressions')
        @self.pg.production('call_signature : GT_CALL expressions')
        @self.pg.production('call_signature : LTEQ_CALL expressions')
        @self.pg.production('call_signature : GTEQ_CALL expressions')
        @self.pg.production(
            'call_signature : NOTEQ_CALL expression expressions')
        @self.pg.production('call_signature : FUNC_CALL expressions')
        @self.pg.production('call_signature : FUNC_BANG expressions')
        @self.pg.production('call_signature : FUNC_PRED expressions')
        def call_signature(state, p):
            return p

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

        @self.pg.production('if : IF expressions')
        def ifexpr(state, p):
            """If parse"""
            i = p.pop(0)
            return ast.If.generate(_flatten_list(p[0].value), i, self.input)

        @self.pg.production('let : LET LBRACKET RBRACKET expression')
        @self.pg.production('let : LET LBRACKET letpairs RBRACKET expression')
        @self.pg.production(
            'let : LET strict_symbol LBRACKET RBRACKET expression')
        @self.pg.production(
            'let : LET strict_symbol LBRACKET letpairs RBRACKET expression')
        def letexpr(state, p):
            """Let parse"""
            return ast.Let.generate(p, self.input)

        @self.pg.production('letpairs : strict_symbol expression')
        @self.pg.production(
            'letpairs : strict_symbol expression COMMA letpairs')
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

        @self.pg.production('collection : empty_collection')
        @self.pg.production('collection : LANGLE expressions RANGLE')
        @self.pg.production('collection : LBRACKET expressions RBRACKET')
        @self.pg.production('collection : LBRACE expressions RBRACE')
        @self.pg.production('collection : LSET expressions RBRACE')
        def collections(state, p):
            if len(p) == 1:
                return p[0]
            else:
                t = p[0]
                x = _collection_type(t)
                del p[2]
                del p[0]
                return ast.Collection(x, p, t, self.input)

        @self.pg.production('empty_collection : empty_vector')
        @self.pg.production('empty_collection : empty_list')
        @self.pg.production('empty_collection : empty_map')
        @self.pg.production('empty_collection : empty_set')
        def empty_collections(state, p):
            return p[0]

        @self.pg.production("empty_vector : LANGLE RANGLE")
        def empty_vector(state, p):
            ct = _collection_type(p[0])
            return ast.EmptyCollection.generate(
                ct,
                p[0],
                self.input)

        @self.pg.production("empty_list : LBRACKET RBRACKET")
        def empty_list(state, p):
            ct = _collection_type(p[0])
            return ast.EmptyCollection.generate(
                ct,
                p[0],
                self.input)

        @self.pg.production("empty_map : LBRACE RBRACE")
        def empty_map(state, p):
            ct = _collection_type(p[0])
            return ast.EmptyCollection.generate(
                ct,
                p[0],
                self.input)

        @self.pg.production("empty_set : LSET RBRACE")
        def empty_set(state, p):
            ct = _collection_type(p[0])
            return ast.EmptyCollection.generate(
                ct,
                p[0],
                self.input)

        @self.pg.production('literal : CHAR')
        @self.pg.production('literal : BIT')
        @self.pg.production('literal : HEX')
        @self.pg.production('literal : INTEGER')
        @self.pg.production('literal : REAL')
        @self.pg.production('literal : STRING')
        @self.pg.production('literal : KEYWORD')
        def literal(state, p):
            return _literal_entry(state.literals, p[0], self.input)

        @self.pg.production("strict_symbols : strict_symbol")
        @self.pg.production("strict_symbols : strict_symbol strict_symbols")
        @self.pg.production(
            "strict_symbols : strict_symbol COMMA strict_symbols")
        def strict_symbols(state, p):
            if len(p) == 1:
                return ast.SymbolList(p)
            else:
                return _token_eater(p, ast.SymbolList)

        @self.pg.production('strict_symbol : SYMBOL')
        @self.pg.production('strict_symbol : SYMBOL_PRED')
        @self.pg.production('strict_symbol : SYMBOL_BANG')
        def strict_symbol(state, p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

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

        @self.pg.production("symbols_only : symbol_only")
        @self.pg.production("symbols_only : symbol_only symbols_only")
        @self.pg.production("symbols_only : symbol_only COMMA symbols_only")
        def symbols_only(state, p):
            if len(p) == 1:
                return ast.SymbolList(p)
            else:
                return _token_eater(p, ast.SymbolList)

        @self.pg.production('symbol_only : SYMBOL')
        def symbol_only(state, p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

    def get_parser(self):
        return self.pg.build()
