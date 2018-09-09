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


class Parser():
    def __init__(self, mlexer, input):
        # A list of all token names accepted by the parser.
        self.pg = ParserGenerator(
            mlexer.get_tokens(),
            precedence=[
                ("left", ["SYMBOL", "KEYWORD"]),
                ("left", ["SYMBOL_BANG"]),
                ("left", ["FUNC_CALL"]),
                ("left", ["FUNC"])])
        self._input = input

    @property
    def input(self):
        return self._input

    def parse(self):
        @self.pg.production('program : module')
        def program(p):
            return ast.CompilationUnit(p)

        @self.pg.production('module : MODULE module_symbol')
        @self.pg.production('module : MODULE module_symbol i_decl')
        @self.pg.production('module : MODULE module_symbol i_decl decltypes')
        @self.pg.production('module : MODULE module_symbol decltypes')
        def mod(p):
            pp = _flatten_list(p)
            return ast.Module(pp[1], pp[2:], pp[0], self.input)

        @self.pg.production('module_symbol : SYMBOL')
        def mod_sym(p):
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production('i_decl : INCLUDE symbol')
        @self.pg.production('i_decl : INCLUDE symbol i_decl')
        @self.pg.production('i_decl : INCLUDE symbol_list')
        @self.pg.production('i_decl : INCLUDE symbol_list i_decl')
        def i_decl(p):
            t = p.pop(0)
            return ast.Include(_symbol_only(p), t, self.input)

        @self.pg.production('decltypes : var_decl')
        @self.pg.production('decltypes : var_decl decltypes')
        @self.pg.production('decltypes : func_decl')
        @self.pg.production('decltypes : func_decl decltypes')
        def decltype(p):
            return _flatten_list(p)

        @self.pg.production('var_decl : VAR var_symbol single_expr')
        @self.pg.production(
            'func_decl : FUNC func_symbol symbol_list')
        @self.pg.production(
            'func_decl : FUNC func_symbol symbol_list multiexpression')
        def v_decl(p):
            t = p.pop(0)
            if t.gettokentype() is 'VAR':
                return ast.Variable(p.pop(0), p, t, self.input)
            else:
                return ast.Function(p.pop(0), p, t, self.input)

        @self.pg.production('func_symbol : symbol_type')
        def func_sym(p):
            return p[0]
            # return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production('var_symbol : symbol_type')
        def var_sym(p):
            return p[0]
            # return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production('symbol_list : LBRACKET RBRACKET')
        @self.pg.production('symbol_list : LBRACKET symbol_seps RBRACKET')
        def symbol_list_1(p):
            if len(p) < 3:
                return ast.EmptyCollection(CollTypes.LIST)
            else:
                return p[1]

        @self.pg.production('symbol_seps : symbol')
        @self.pg.production('symbol_seps : symbol symbol_seps')
        @self.pg.production('symbol_seps : symbol COMMA symbol_seps')
        def symbol_list_2(p):
            if len(p) == 1:
                return ast.SymbolList(p)
            else:
                return _token_eater(p, ast.SymbolList)

        @self.pg.production('multiexpression : simple_expr')
        @self.pg.production('multiexpression : complex_expr')
        @self.pg.production('multiexpression : simple_expr multiexpression')
        def simple_expr_p(p):
            """Parse one or more expressions"""
            # print("multiple = {}".format(p))
            return ast.Expressions(p)

        @self.pg.production('single_expr : simple_expr')
        @self.pg.production('single_expr : complex_expr')
        def singleexpr_p(p):
            """Parse one expressions"""
            # print("single expression = {}".format(p))
            return p[0]

        @self.pg.production('complex_expr : functioncall')
        @self.pg.production('complex_expr : letexpr')
        @self.pg.production('complex_expr : matchexpr')
        @self.pg.production('complex_expr : partialexpr')
        @self.pg.production('complex_expr : groupexpr')
        @self.pg.production('complex_expr : lambdaexpr')
        def complex_expr(p):
            """Expression parse"""
            # print("complex expression = {}".format(p))
            return ast.Expression(p)

        @self.pg.production('simple_expr : literal')
        @self.pg.production('simple_expr : symbol')
        @self.pg.production('simple_expr : empty_collection')
        @self.pg.production('simple_expr : collection')
        def simple_expr(p):
            """Expression parse"""
            # print("simple expression = {}".format(p))
            return ast.Expression(p)

        @self.pg.production('functioncall : FUNC_CALL multiexpression')
        @self.pg.production('functioncall : FUNC_BANG multiexpression')
        @self.pg.production('functioncall : FUNC_PRED multiexpression')
        def functioncall(p):
            # print("func = {}".format(p))
            fcall = ast.FunctionCall(p.pop(0).getstr(), p)
            return fcall

        @self.pg.production('groupexpr : GROUP RPAREN')
        @self.pg.production('groupexpr : GROUP multiexpression RPAREN')
        def group(p):
            """Group parse for zero or more expressions"""
            if len(p) < 3:
                return ast.Group([])
            else:
                del p[2]
                del p[0]
                return ast.Group(p)

        # TODO: Stack frame context
        @self.pg.production(
            'letexpr : LET LBRACKET RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET LBRACKET letpairs RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET symbol LBRACKET RBRACKET single_expr')
        @self.pg.production(
            'letexpr : LET symbol LBRACKET letpairs RBRACKET single_expr')
        def letexpr(p):
            """Let parse"""
            letset = [x for x in p if type(x) is not Token]
            return ast.Let(letset)

        @self.pg.production('letpairs : symbol single_expr')
        @self.pg.production('letpairs : symbol single_expr COMMA letpairs')
        def letpairs(p):
            """Let parse support for zero or more local var assignments"""
            return _token_eater(p, ast.LetPairs)

        @self.pg.production('matchexpr : MATCH simple_expr')
        # @self.pg.production('matchexpr : MATCH simple_expr matchpairs')
        def matchexpr(p):
            """Match parse"""
            return ast.Match([x for x in p if type(x) is not Token])

        @self.pg.production('matchpairs : simple_expr simple_expr')
        @self.pg.production('matchpairs : simple_expr simple_expr matchpairs')
        def matchpairs(p):
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

        @self.pg.production('lambdaexpr : LAMBDA symbol_list simple_expr')
        def lambdaexpr(p):
            """Lambda parse"""
            p.pop(0)
            return ast.Lambda(p)

        @self.pg.production('partialexpr : LPAREN simple_expr RPAREN')
        def partialexpr(p):
            """Partial parse"""
            p.pop(2)
            p.pop(0)
            return ast.Partial(p)

        @self.pg.production('empty_collection : LANGLE RANGLE')
        @self.pg.production('empty_collection : LBRACKET RBRACKET')
        @self.pg.production('empty_collection : LBRACE RBRACE')
        @self.pg.production('empty_collection : LSET RBRACE')
        def empty_collections(p):
            return ast.EmptyCollection(_collection_type(p[0]))

        # TODO: Add map constrains of even number expressions
        @self.pg.production('collection : LANGLE simple_expr_seps RANGLE')
        @self.pg.production('collection : LBRACKET simple_expr_seps RBRACKET')
        @self.pg.production('collection : LBRACE simple_expr_seps RBRACE')
        @self.pg.production('collection : LSET simple_expr_seps RBRACE')
        def collections(p):
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
        def expression_list(p):
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
        def literal(p):
            # print("Literal => {}".format(p))
            return ast.Literal(
                p[0].gettokentype(),
                p[0].getstr(),
                p[0],
                self.input)

        @self.pg.production('symbol_type : SYMBOL')
        @self.pg.production('symbol_type : SYMBOL_BANG')
        @self.pg.production('symbol_type : SYMBOL_PRED')
        def symbol_type(p):
            # state.resolve_symbol(p)
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.production('symbol : SYMBOL')
        def symbol(p):
            # state.resolve_symbol(p)
            return ast.Symbol(p[0].getstr(), p[0], self.input)

        @self.pg.error
        def error_handle(token):
            raise ValueError(
                "{} at {} in {} where it wasn't expected".format(
                    token,
                    self.input,
                    token.getsourcepos()))

    def get_parser(self):
        return self.pg.build()
