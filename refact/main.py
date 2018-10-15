#!/usr/bin/env python3
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

import sys
from lexer import Lexer
from rply import ParserGenerator


class Parser():
    def __init__(self, mlexer, input):
        self.pg = ParserGenerator(
            mlexer.get_tokens(),
            precedence=[])
        self._input = input

    @property
    def input(self):
        return self._input

    def parse(self):
        @self.pg.production('program : module')
        def program(state, p):
            return p

        @self.pg.production('module : MODULE symbol')
        @self.pg.production('module : MODULE symbol include_declaration')
        @self.pg.production(
            'module : MODULE symbol include_declaration declarations')
        @self.pg.production('module : MODULE symbol declarations')
        def module(state, p):
            p.pop(0)
            name = p.pop(0)
            return {
                "type": "module",
                "name": name,
                "value": p}
            # return p

        @self.pg.production('include_declaration : INCLUDE symbol')
        @self.pg.production(
            'include_declaration : INCLUDE symbol include_declaration')
        @self.pg.production('include_declaration : INCLUDE symbol_list')
        @self.pg.production(
            'include_declaration : INCLUDE symbol_list include_declaration')
        def include_declaration(state, p):
            return p

        @self.pg.production('declarations : func_declaration')
        @self.pg.production('declarations : var_declaration')
        @self.pg.production('declarations : func_declaration declarations')
        @self.pg.production('declarations : var_declaration declarations')
        def declarations(state, p):
            return p

        @self.pg.production(
            'func_declaration : FUNC strict_symbol symbol_list')
        @self.pg.production(
            'func_declaration : FUNC KEYWORD strict_symbol symbol_list')
        @self.pg.production(
            'func_declaration : FUNC strict_symbol symbol_list expressions')
        @self.pg.production(
            'func_declaration : FUNC KEYWORD strict_symbol symbol_list expressions')
        def func_declarations(state, p):
            p.pop(0)
            return {
                "type": "Function",
                "name": p.pop(0),
                "args": p.pop(0),
                "value": p}
            # return p

        @self.pg.production('var_declaration : VAR strict_symbol')
        @self.pg.production('var_declaration : VAR KEYWORD strict_symbol')
        @self.pg.production('var_declaration : VAR strict_symbol expression')
        @self.pg.production(
            'var_declaration : VAR KEYWORD strict_symbol expression')
        def var_declarations(state, p):
            p.pop(0)
            return {
                "type": "Variable",
                "name": p.pop(0),
                "value": p}

        @self.pg.production('expression : MATH_REF')
        @self.pg.production('expression : symbol')
        @self.pg.production('expression : literal')
        @self.pg.production('expression : collection')
        @self.pg.production('expression : function_call')
        @self.pg.production('expression : group')
        @self.pg.production('expression : lambda')
        @self.pg.production('expression : if')
        @self.pg.production('expression : partial')
        @self.pg.production('expression : let')
        @self.pg.production('expression : match')
        def expression(state, p):
            return p
            # return {
            #     "type": "Expression",
            #     "value": p}

        @self.pg.production('expressions : expression COMMA expressions')
        @self.pg.production('expressions : expression expressions')
        @self.pg.production('expressions : expression')
        def expressions(state, p):
            return {
                "type": "Expressions",
                "value": p}

        @self.pg.production('function_call : call_signature')
        # @self.pg.production('function_call : expression')
        def function_call(state, p):
            return p

        @self.pg.production('call_signature : MATH_CALL expression expression')
        @self.pg.production('call_signature : EQ_CALL expression expression')
        @self.pg.production('call_signature : LT_CALL expression expression')
        @self.pg.production('call_signature : GT_CALL expression expression')
        @self.pg.production('call_signature : LTEQ_CALL expression expression')
        @self.pg.production('call_signature : GTEQ_CALL expression expression')
        @self.pg.production(
            'call_signature : NOTEQ_CALL expression expression')
        @self.pg.production('call_signature : FUNC_CALL func_expressions')
        @self.pg.production('call_signature : FUNC_BANG func_expressions')
        @self.pg.production('call_signature : FUNC_PRED func_expressions')
        def call_signature(state, p):
            return {
                "type": "Call",
                "name": p.pop(0).getstr(),
                "value": p}

        @self.pg.production('func_expressions : expression')
        def func_expressions(state, p):
            return p

        @self.pg.production('group : GROUP RPAREN')
        @self.pg.production('group : GROUP expressions RPAREN')
        def group(state, p):
            """Group parse for zero or more expressions"""
            return {
                "type": p.pop(0).gettokentype(),
                "value": p}

        @self.pg.production('lambda : LAMBDA symbol_list expression')
        def lambdaexpr(state, p):
            """Lambda parse"""
            return {
                "type": p.pop(0).gettokentype(),
                "value": p}

        @self.pg.production('partial : LPAREN expression RPAREN')
        def partial(state, p):
            """Partial parse"""
            return {
                "type": "Partial",
                "value": p}

        @self.pg.production('if : IF expression expression expression')
        def ifexpr(state, p):
            # print('if expr: {}'.format(p))
            return {
                "type": p.pop(0).gettokentype(),
                "value": p}

        @self.pg.production('let : LET LBRACKET RBRACKET expression')
        @self.pg.production('let : LET LBRACKET letpairs RBRACKET expression')
        @self.pg.production(
            'let : LET strict_symbol LBRACKET RBRACKET expression')
        @self.pg.production(
            'let : LET strict_symbol LBRACKET letpairs RBRACKET expression')
        def letexpr(state, p):
            """Let parse"""
            return {
                "type": p.pop(0).gettokentype(),
                "value": p}

        @self.pg.production('letpairs : strict_symbol expression')
        @self.pg.production(
            'letpairs : strict_symbol expression COMMA letpairs')
        def letpairs(state, p):
            """Let parse support for zero or more local var assignments"""
            return {
                "type": "Letpair",
                "value": p}

        @self.pg.production('match : MATCH expression matchpairs')
        @self.pg.production(
            'match : MATCH strict_symbol expression matchpairs')
        def match(state, p):
            """Match parse"""
            return {
                "type": "Match",
                "value": p}

        @self.pg.production(
            'matchpairs : MATCH_PATTERN expression expression')
        @self.pg.production(
            'matchpairs : MATCH_PATTERN expression expression matchpairs')
        @self.pg.production(
            'matchpairs : MATCH_PATTERN MATCH_DEFAULT expression')
        def matchpairs(state, p):
            return {
                "type": p.pop(0).gettokentype(),
                "value": p}

        @self.pg.production("symbol_list : empty_list")
        @self.pg.production("symbol_list : LBRACKET strict_symbols RBRACKET")
        def symbol_list(state, p):
            return p[0]

        @self.pg.production('collection : empty_collection')
        @self.pg.production('collection : LANGLE expressions RANGLE')
        @self.pg.production('collection : LBRACKET expressions RBRACKET')
        @self.pg.production('collection : LBRACE expressions RBRACE')
        @self.pg.production('collection : LSET expressions RBRACE')
        def collections(state, p):
            return p

        @self.pg.production('empty_collection : empty_vector')
        @self.pg.production('empty_collection : empty_list')
        @self.pg.production('empty_collection : empty_map')
        @self.pg.production('empty_collection : empty_set')
        def empty_collections(state, p):
            return p

        @self.pg.production("empty_vector : LANGLE RANGLE")
        def empty_vector(state, p):
            return p

        @self.pg.production("empty_list : LBRACKET RBRACKET")
        def empty_list(state, p):
            return p

        @self.pg.production("empty_map : LBRACE RBRACE")
        def empty_map(state, p):
            return p

        @self.pg.production("empty_set : LSET RBRACE")
        def empty_set(state, p):
            return p

        @self.pg.production('literal : CHAR')
        @self.pg.production('literal : BIT')
        @self.pg.production('literal : HEX')
        @self.pg.production('literal : INTEGER')
        @self.pg.production('literal : REAL')
        @self.pg.production('literal : STRING')
        @self.pg.production('literal : KEYWORD')
        def literal(state, p):
            return {"type": "Literal", "value": p.pop(0).getstr()}

        @self.pg.production("strict_symbols : strict_symbol")
        @self.pg.production("strict_symbols : strict_symbol strict_symbols")
        @self.pg.production(
            "strict_symbols : strict_symbol COMMA strict_symbols")
        def strict_symbols(state, p):
            return {"type": "Strict_Symbols", "value": p}

        @self.pg.production('strict_symbol : SYMBOL')
        @self.pg.production('strict_symbol : SYMBOL_PRED')
        @self.pg.production('strict_symbol : SYMBOL_BANG')
        def strict_symbol(state, p):
            return {"type": "Strict_Symbol", "value": p[0]}

        @self.pg.production('symbol : MATCH_EXPRREF')
        def matchexprref(state, p):
            return {"type": p[0].gettokentype(), "value": p}

        @self.pg.production('symbol : IF_REF')
        @self.pg.production('symbol : EQ_REF')
        @self.pg.production('symbol : LT_REF')
        @self.pg.production('symbol : GT_REF',)
        @self.pg.production('symbol : LTEQ_REF')
        @self.pg.production('symbol : GTEQ_REF')
        @self.pg.production('symbol : NOTEQ_REF')
        @self.pg.production('symbol : SYMBOL')
        def symbol(state, p):
            return {"type": "Symbol", "value": p[0]}

    def get_parser(self):
        return self.pg.build()


def main(prog_name=sys.argv[0], args=sys.argv[1:]):
    """Main entry point for parser bootstrap"""
    if not args:
        args.append('-h')
    else:
        pass

    srcfile = "test.foidl"
    try:
        with open(srcfile, 'rt') as fsrc:
            src = fsrc.read()
    except OSError as err:
        raise("OS error: {}".format(err))

    lgen = Lexer()
    lexer = lgen.get_lexer()
    tokens = lexer.lex(src)
    # [print(x) for x in tokens]
    pargen = Parser(lgen, "foo")
    pargen.parse()
    parser = pargen.get_parser()
    print(parser.parse(tokens, state={}))


if __name__ == "__main__":
    main()
