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
import ast


def _collection_type(p0):
    x = None
    if p0.gettokentype() is 'LSET':
        x = ast.CollTypes.SET
    elif p0.gettokentype() is 'LBRACKET':
        x = ast.CollTypes.LIST
    elif p0.gettokentype() is 'LBRACE':
        x = ast.CollTypes.MAP
    else:
        x = ast.CollTypes.VECTOR
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


class Parser():
    def __init__(self, mlexer):
        # A list of all token names accepted by the parser.
        self.pg = ParserGenerator(
            mlexer.get_tokens(),
            precedence=[])
        self.cscope = ast.CompilationScope()

    def parse(self):
        @self.pg.production('program : module')
        def program(p):
            return self.cscope.build_unit(p)

        @self.pg.production('module : MODULE symbol')
        @self.pg.production('module : MODULE symbol i_decl')
        @self.pg.production('module : MODULE symbol i_decl decltypes')
        @self.pg.production('module : MODULE symbol decltypes')
        def mod(p):
            p.pop(0)
            return ast.Module(p.pop(0), p)

        @self.pg.production('i_decl : INCLUDE symbol')
        @self.pg.production('i_decl : INCLUDE symbol i_decl')
        @self.pg.production('i_decl : INCLUDE symbol_list')
        @self.pg.production('i_decl : INCLUDE symbol_list i_decl')
        def i_decl(p):
            return ast.Include(p)

        @self.pg.production('decltypes : var_decl')
        @self.pg.production('decltypes : var_decl decltypes')
        @self.pg.production('decltypes : func_decl')
        @self.pg.production('decltypes : func_decl decltypes')
        def decltype(p):
            print("decltypes = {}".format(p))
            return p[0]

        @self.pg.production('var_decl : VAR symbol sexpr')
        def v_decl(p):
            t = p.pop(0)
            if t.gettokentype() is 'VAR':
                print("IS VAR")
            else:
                print("IS NOT VAR")
            return ast.Variable(p.pop(0), p)

        @self.pg.production('func_decl : FUNC symbol symbol_list sexpressions')
        def f_decl(p):
            del p[0]
            return ast.Function(p.pop(0), p)

        @self.pg.production('symbol_list : LBRACKET RBRACKET')
        @self.pg.production('symbol_list : LBRACKET symbol_seps RBRACKET')
        def symbol_list_1(p):
            if len(p) < 3:
                return ast.EmptyCollection(ast.CollTypes.LIST)
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

        @self.pg.production('sexpressions : sexpr')
        # @self.pg.production('sexpressions : sexpr sexpressions')
        def sexpr_p(p):
            """Parse one or more expressions"""
            print("sexpressions = {}".format(p))
            return ast.Expressions(p)

        @self.pg.production('sexpr : literal')
        @self.pg.production('sexpr : symbol')
        @self.pg.production('sexpr : empty_collection')
        @self.pg.production('sexpr : collection')
        @self.pg.production('sexpr : functioncall')
        @self.pg.production('sexpr : letexpr')
        @self.pg.production('sexpr : matchexpr')
        @self.pg.production('sexpr : partialexpr')
        @self.pg.production('sexpr : grpexpr')
        @self.pg.production('sexpr : lambdaexpr')
        def sexpr(p):
            """Expression parse"""
            print("sexpr = {}".format(p))
            return ast.Expression(p)

        @self.pg.production('grpexpr : GROUP RPAREN')
        @self.pg.production('grpexpr : GROUP sexpressions RPAREN')
        def group(p):
            """Group parse for zero or more expressions"""
            if len(p) < 3:
                return ast.Group([])
            else:
                del p[2]
                del p[0]
                return ast.Group(p)

        # TODO: Stack frame context
        @self.pg.production('letexpr : LET LBRACKET RBRACKET sexpr')
        @self.pg.production('letexpr : LET LBRACKET letpairs RBRACKET sexpr')
        @self.pg.production('letexpr : LET symbol LBRACKET RBRACKET sexpr')
        @self.pg.production(
            'letexpr : LET symbol LBRACKET letpairs RBRACKET sexpr')
        def letexpr(p):
            """Let parse"""
            return ast.Let([x for x in p if type(x) is not Token])

        @self.pg.production('letpairs : symbol sexpr')
        @self.pg.production('letpairs : symbol sexpr letpairs')
        @self.pg.production('letpairs : symbol sexpr COMMA letpairs')
        def letpairs(p):
            """Let parse support for zero or more local var assignments"""
            return _token_eater(p, ast.LetPairs)

        @self.pg.production('matchexpr : MATCH sexpr')
        # @self.pg.production('matchexpr : MATCH sexpr matchpairs')
        def matchexpr(p):
            """Match parse"""
            return ast.Match([x for x in p if type(x) is not Token])

        @self.pg.production('matchpairs : sexpr sexpr')
        @self.pg.production('matchpairs : sexpr sexpr matchpairs')
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

        @self.pg.production('lambdaexpr : LAMBDA symbol_list sexpr')
        def lambdaexpr(p):
            """Lambda parse"""
            p.pop(0)
            return ast.Lambda(p)

        @self.pg.production('partialexpr : LPAREN sexpr RPAREN')
        def partialexpr(p):
            """Partial parse"""
            p.pop(2)
            p.pop(0)
            return ast.Partial(p)

        @self.pg.production('functioncall : FUNC_CALL sexpressions')
        def functioncall(p):
            x = p.pop(0)
            print("*********** CALL to {} {}".format(x.getstr(), p))
            p[0].eval()
            return ast.FunctionCall(p)

        @self.pg.production('empty_collection : LANGLE RANGLE')
        @self.pg.production('empty_collection : LBRACKET RBRACKET')
        @self.pg.production('empty_collection : LBRACE RBRACE')
        @self.pg.production('empty_collection : LSET RBRACE')
        def empty_collections(p):
            return ast.EmptyCollection(_collection_type(p[0]))

        # TODO: Add map constrains of even number expressions
        @self.pg.production('collection : LANGLE sexpr_seps RANGLE')
        @self.pg.production('collection : LBRACKET sexpr_seps RBRACKET')
        @self.pg.production('collection : LBRACE sexpr_seps RBRACE')
        @self.pg.production('collection : LSET sexpr_seps RBRACE')
        def collections(p):
            if len(p) < 3:
                return self.empty_collection(p)
            else:
                x = _collection_type(p[0])
                del p[2]
                del p[0]
                return ast.Collection(x, p)

        @self.pg.production('sexpr_seps : sexpr')
        @self.pg.production('sexpr_seps : sexpr sexpr_seps')
        @self.pg.production('sexpr_seps : sexpr COMMA sexpr_seps')
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
            return self.cscope.build_literal(ast.Literal(
                p[0].gettokentype(),
                p[0].getstr()))

        @self.pg.production('symbol : SYMBOL')
        def symbol(p):
            return ast.Symbol(p[0].getstr())

        @self.pg.error
        def error_handle(token):
            raise ValueError(
                "%s where it wasn't expected" % token)

    def get_parser(self):
        return self.pg.build()
