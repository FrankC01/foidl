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

from rply import ParserGenerator
from ast import Number, Sum, Sub, Print


class Parser():
    def __init__(self, mlex, module, builder, printf):
        # A list of all token names accepted by the parser.
        self.pg = ParserGenerator(
            mlex.get_tokens(),
            precedence=[
                ('left', ['SUM', 'SUB'])])
        self.module = module
        self.builder = builder
        self.printf = printf

    def parse(self):
        @self.pg.production(
            # 'program : PRINT expr SEMI_COLON')
            'program : PRINT OPEN_PAREN  expr CLOSE_PAREN SEMI_COLON')
        def program(p):
            return Print(self.builder, self.module, self.printf, p[2])

        # @self.pg.production('expr : expr SUM expr')
        @self.pg.production('expr : expr SUM expr')
        @self.pg.production('expr : expr SUB expr')
        def math_expr(p):
            left = p[0]
            operator = p[1]
            right = p[2]
            if operator.gettokentype() == 'SUB':
                return Sub(self.builder, self.module, left, right)
            else:
                return Sum(self.builder, self.module, left, right)

        # @self.pg.production('expression : expression SUM expression')
        # @self.pg.production('expression : expression SUB expression')
        # def expression(p):
        #     left = p[0]
        #     right = p[2]
        #     operator = p[1]
        #     if operator.gettokentype() == 'SUM':
        #         return Sum(left, right)
        #     elif operator.gettokentype() == 'SUB':
        #         return Sub(left, right)

        @self.pg.production('expr : NUMBER')
        def number(p):
            return Number(self.builder, self.module, p[0].value)

        @self.pg.error
        def error_handle(token):
            raise ValueError(
                "Ran into a %s where it wasn't expected" % token.gettokentype())

    def get_parser(self):
        return self.pg.build()
