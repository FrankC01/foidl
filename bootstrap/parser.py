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


def _symbol_only(in_list, out_list=None):
    if not out_list:
        out_list = []
    for n in in_list:
        if type(n) is ast.Symbol:
            out_list.append(n)
        else:
            out_list = _symbol_only(n.value, out_list)
    return out_list


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


class Parser():
    def __init__(self, mlexer, input):
        # A list of all token names accepted by the parser.
        self.pg = ParserGenerator(
            mlexer.get_tokens(),
            # precedence=[])
            precedence=[
                ("left", ["SYMBOL", "KEYWORD"]),
                ("left", ["SYMBOL_BANG", "SYMBOL_PRED"]),
                ("left", ["FUNC_CALL", "FUNC_BANG", "FUNC_PRED"]),
                ("left", ["FUNC", "VAR"])])
        self._input = input

    @property
    def input(self):
        return self._input

    def parse(self):
        @self.pg.production('program : module')
        def program(state, p):
            return ast.CompilationUnit(p)

        @self.pg.production('module : MODULE symbol')
        @self.pg.production('module : MODULE symbol i_decl')
        @self.pg.production('module : MODULE symbol i_decl decltypes')
        @self.pg.production('module : MODULE symbol decltypes')
        def mod(state, p):
            pp = _flatten_list(p)
            return ast.Module(pp[1], pp[2:], pp[0], self.input)

        @self.pg.production('i_decl : INCLUDE symbol')
        @self.pg.production('i_decl : INCLUDE symbol i_decl')
        @self.pg.production('i_decl : INCLUDE symbol_list')
        @self.pg.production('i_decl : INCLUDE symbol_list i_decl')
        def i_decl(state, p):
            if state.incprocessed:
                raise errors.IncludeOutOfOrder(
                    "Include expression found after declarations")
            t = p.pop(0)
            i = ast.Include(_symbol_only(p), t, self.input)
            include_parse(state, i)
            return i

        @self.pg.production('decltypes : var_decl')
        @self.pg.production('decltypes : var_decl decltypes')
        @self.pg.production('decltypes : func_decl')
        @self.pg.production('decltypes : func_decl decltypes')
        def decltype(state, p):
            return _flatten_list(p)

        @self.pg.production('var_decl : var_header single_expr')
        @self.pg.production('func_decl : func_header')
        @self.pg.production('func_decl : func_header multiexpression')
        def v_decl(state, p):
            t = p.pop(0)
            if type(t) == ast.VarHeader:
                return ast.Variable(t.name, p, t.token, self.input)
            else:
                return ast.Function(t, p, self.input)

        @self.pg.production('var_header : VAR decl_symbol')
        def var_hdr(state, p):
            t = p.pop(0)
            vh = ast.VarHeader(p.pop(0), t, self.input)
            if not state.incprocessed and state.mainsrc == self.input:
                state.incprocessed = True
                state.symtree.push_scope(self.input, self.input)
            state.symtree.register_symbol(vh.name.value, vh.get_reference())
            return vh

        @self.pg.production('func_header : FUNC decl_symbol fsymbol_list')
        def func_hdr(state, p):
            t = p.pop(0)
            fh = ast.FuncHeader(p.pop(0), p[0], t, self.input)
            if not state.incprocessed and state.mainsrc == self.input:
                state.incprocessed = True
                state.symtree.push_scope(self.input, self.input)
            state.symtree.register_symbol(fh.name.value, fh.get_reference())
            return fh

        @self.pg.production('decl_symbol : symbol_type')
        def decl_sym(state, p):
            return p[0]

        @self.pg.production('symbol_list : LBRACKET RBRACKET')
        @self.pg.production('symbol_list : LBRACKET symbol_seps RBRACKET')
        def symbol_list_1(state, p):
            if len(p) < 3:
                return ast.EmptyCollection(
                    CollTypes.LIST,
                    [],
                    p[0],
                    self.input)
            else:
                return p[1]

        @self.pg.production('symbol_seps : symbol')
        @self.pg.production('symbol_seps : symbol symbol_seps')
        @self.pg.production('symbol_seps : symbol COMMA symbol_seps')
        def symbol_list_2(state, p):
            if len(p) == 1:
                return ast.SymbolList(p)
            else:
                return _token_eater(p, ast.SymbolList)

        @self.pg.production('fsymbol_list : LBRACKET RBRACKET')
        @self.pg.production('fsymbol_list : LBRACKET fsymbol_seps RBRACKET')
        def fsymbol_list_1(state, p):
            if len(p) < 3:
                return ast.EmptyCollection(
                    CollTypes.LIST,
                    [],
                    p[0],
                    self.input)
            else:
                return p[1]

        @self.pg.production('fsymbol_seps : symbol_type')
        @self.pg.production('fsymbol_seps : symbol_type fsymbol_seps')
        @self.pg.production('fsymbol_seps : symbol_type COMMA fsymbol_seps')
        def fsymbol_list_2(state, p):
            if len(p) == 1:
                return ast.SymbolList(p)
            else:
                return _token_eater(p, ast.SymbolList)

        @self.pg.production('multiexpression : single_expr')
        @self.pg.production('multiexpression : single_expr multiexpression')
        def simple_expr_p(state, p):
            """Parse one or more expressions"""
            if len(p) == 1 and type(p[0]) is list:
                p = p[0]
            t = _token_eater(p, ast.Expressions)
            # print("ME => {} -> {}".format(p, t.value))
            return t

        @self.pg.production('single_expr : simple_expr')
        @self.pg.production('single_expr : complex_expr')
        def singleexpr_p(state, p):
            """Parse one expressions"""
            # print("single expression = {}".format(p))
            return p[0]

        @self.pg.production('complex_expr : functioncall')
        @self.pg.production('complex_expr : if_expr')
        @self.pg.production('complex_expr : letexpr')
        @self.pg.production('complex_expr : matchexpr')
        @self.pg.production('complex_expr : partialexpr')
        @self.pg.production('complex_expr : groupexpr')
        @self.pg.production('complex_expr : lambdaexpr')
        def complex_expr(state, p):
            """Expression parse"""
            # print("complex expression = {}".format(p))
            return p[0]
            # return ast.Expression(p)

        @self.pg.production('simple_expr : literal')
        @self.pg.production('simple_expr : symbol_type')
        @self.pg.production('simple_expr : empty_collection')
        @self.pg.production('simple_expr : collection')
        def simple_expr(state, p):
            """Expression parse"""
            # print("simple expression = {}".format(p))
            return p[0]
            # return ast.Expression(p)

        @self.pg.production('functioncall : callsig')
        @self.pg.production('functioncall : callsig multiexpression')
        def functioncall(state, p):
            t = p.pop(0)
            fcall = ast.FunctionCall.re_factor(
                t.getstr(), p, t, self.input, state)
            # fcall = ast.FunctionCall(t.getstr(), p, t, self.input, state)
            return fcall

        @self.pg.production('callsig : FUNC_CALL')
        @self.pg.production('callsig : FUNC_BANG')
        @self.pg.production('callsig : FUNC_PRED')
        @self.pg.production('callsig : EQ_CALL')
        @self.pg.production('callsig : LT_CALL')
        @self.pg.production('callsig : GT_CALL')
        @self.pg.production('callsig : LTEQ_CALL')
        @self.pg.production('callsig : GTEQ_CALL')
        @self.pg.production('callsig : NOTEQ_CALL')
        @self.pg.production('callsig : ADD_CALL')
        @self.pg.production('callsig : SUB_CALL')
        @self.pg.production('callsig : MUL_CALL')
        @self.pg.production('callsig : DIV_CALL')
        def callsig(state, p):
            return p[0]

        @self.pg.production('groupexpr : GROUP RPAREN')
        @self.pg.production('groupexpr : GROUP multiexpression RPAREN')
        def group(state, p):
            """Group parse for zero or more expressions"""
            if len(p) < 3:
                return ast.Group([])
            else:
                del p[2]
                del p[0]
                return ast.Group(p)

        @self.pg.production(
            'letexpr : LET LBRACKET RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET LBRACKET letpairs RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET symbol_type LBRACKET RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET symbol_type LBRACKET letpairs RBRACKET single_expr')
        def letexpr(state, p):
            """Let parse"""
            return ast.Let.re_factor(p, self.input)

        @self.pg.production('letpairs : symbol_type single_expr')
        @self.pg.production(
            'letpairs : symbol_type single_expr COMMA letpairs')
        def letpairs(state, p):
            """Let parse support for zero or more local var assignments"""
            return _token_eater(_flatten_list(p), ast.LetPairs)

        @self.pg.production('matchexpr : MATCH simple_expr')
        # @self.pg.production('matchexpr : MATCH simple_expr matchpairs')
        def matchexpr(state, p):
            """Match parse"""
            return ast.Match([x for x in p if type(x) is not Token])

        @self.pg.production('matchpairs : simple_expr simple_expr')
        @self.pg.production('matchpairs : simple_expr simple_expr matchpairs')
        def matchpairs(state, p):
            """Match parse support for zero or more match patterns"""
            def eater(in_list):
                out_list = []
                for n in in_list:
                    if type(n) is not Token:
                        if type(n) is ast.MatchPairs:
                            out_list.extend(eater(n.value))
                        else:
                            out_list.append(n)
                    else:
                        pass
                return out_list
            y = eater(p)
            return ast.MatchPairs(y)

        @self.pg.production('lambdaexpr : LAMBDA fsymbol_list simple_expr')
        def lambdaexpr(state, p):
            """Lambda parse"""
            p.pop(0)
            return ast.Lambda(p)

        @self.pg.production('partialexpr : LPAREN simple_expr RPAREN')
        def partialexpr(state, p):
            """Partial parse"""
            p.pop(2)
            p.pop(0)
            return ast.Partial(p)

        @self.pg.production('if_expr : IF single_expr')
        def ifexpr(state, p):
            # print('if expr: {}'.format(p))
            i = p.pop(0)
            return ast.If(p, i, self.input)

        @self.pg.production('empty_collection : LANGLE RANGLE')
        @self.pg.production('empty_collection : LBRACKET RBRACKET')
        @self.pg.production('empty_collection : LBRACE RBRACE')
        @self.pg.production('empty_collection : LSET RBRACE')
        def empty_collections(state, p):
            ct = _collection_type(p[0])
            return ast.EmptyCollection.get_empty_collection(
                ct,
                p[0],
                self.input)

        # TODO: Add map constrains of even number expressions
        @self.pg.production('collection : LANGLE simple_expr_seps RANGLE')
        @self.pg.production('collection : LBRACKET simple_expr_seps RBRACKET')
        @self.pg.production('collection : LBRACE simple_expr_seps RBRACE')
        @self.pg.production('collection : LSET simple_expr_seps RBRACE')
        def collections(state, p):
            if len(p) < 3:
                return self.empty_collection(p)
            else:
                x = _collection_type(p[0])
                del p[2]
                del p[0]
                return ast.Collection(x, p)

        @self.pg.production('simple_expr_seps : simple_expr')
        @self.pg.production('simple_expr_seps : simple_expr simple_expr_seps')
        @self.pg.production(
            'simple_expr_seps : simple_expr COMMA simple_expr_seps')
        def expression_list(state, p):
            """Parse collection expressions"""
            if len(p) == 1:
                return ast.ExpressionList(p)
            else:
                return _token_eater(p, ast.ExpressionList)

        @self.pg.production('literal : CHAR')
        @self.pg.production('literal : BIT')
        @self.pg.production('literal : HEX')
        @self.pg.production('literal : INTEGER')
        @self.pg.production('literal : REAL')
        @self.pg.production('literal : STRING')
        @self.pg.production('literal : KEYWORD')
        def literal(state, p):
            return _literal_entry(state.literals, p[0], self.input)

        @self.pg.production('symbol_type : SYMBOL')
        @self.pg.production('symbol_type : SYMBOL_BANG')
        @self.pg.production('symbol_type : SYMBOL_PRED')
        def symbol_type(state, p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production(' symbol : SYMBOL')
        def symbol(state, p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.error
        def error_handle(state, token):
            raise ValueError(
                "{} at {} in {} where it wasn't expected".format(
                    token,
                    self.input,
                    token.getsourcepos()))

    def get_parser(self):
        return self.pg.build()
