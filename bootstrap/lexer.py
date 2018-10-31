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

from rply import LexerGenerator


class Lexer():
    def __init__(self):
        self.lexer = LexerGenerator()

    def _add_tokens(self):
        # Keywords
        self.lexer.add('MODULE', r'\bmodule\b')
        self.lexer.add('INCLUDE', r'\binclude\b')
        self.lexer.add('PRIVATE', r'\b:private')
        self.lexer.add('VAR', r'\bvar\b')
        self.lexer.add('FUNC', r'\bfunc\b')
        self.lexer.add('LET', r'\blet\b')
        self.lexer.add('MATCH', r'\bmatch\b')
        self.lexer.add('MATCH_DEFAULT', r'\b:default\b')
        # self.lexer.add('PRIVATE', r':private')
        self.lexer.add('LAMBDA', r"\^")
        self.lexer.add('MATCH_GUARD', r'\|')
        self.lexer.add('MATCH_EXPRREF', r'%0')
        self.lexer.add('GROUP', r'@\(')
        self.lexer.add('IF', r'\?:')
        # Boolean operators
        self.lexer.add('EQ_CALL', r"=:")
        self.lexer.add('LT_CALL', r"<:")
        self.lexer.add('GT_CALL', r">:")
        self.lexer.add('LTEQ_CALL', r"<=:")
        self.lexer.add('GTEQ_CALL', r">=:")
        self.lexer.add('NOTEQ_CALL', r"not=:")
        # self.lexer.add('EQ_REF', r"eq")
        # self.lexer.add('LT_REF', r"lt")
        # self.lexer.add('GT_REF', r"gt")
        # self.lexer.add('LTEQ_REF', r"lteq")
        # self.lexer.add('GTEQ_REF', r"gteq")
        # self.lexer.add('NOTEQ_REF', r"neq")
        # Modifiers
        self.lexer.add('AMP', r'&')
        # self.lexer.add('COMMA', r',')
        # Parenthesis
        self.lexer.add('LSET', r"#{")
        self.lexer.add('LPAREN', r"\(")
        self.lexer.add('RPAREN', r"\)")
        # Angles vectors
        self.lexer.add('LANGLE', r'\<')
        self.lexer.add('RANGLE', r'\>')
        # Brackets (symbol lists and lists)
        self.lexer.add('LBRACKET', r'\[')
        self.lexer.add('RBRACKET', r'\]')
        # Sets and maps
        self.lexer.add('LBRACE', r'\{')
        self.lexer.add('RBRACE', r'\}')
        # Semi Colon
        # Strings
        # self.lexer.add('STRING', r'\"(.+?)\"')
        self.lexer.add('STRING', r'"([^"]|\n)*"')
        self.lexer.add('CHAR', r'\'(.{1})\'')
        # Numbers
        self.lexer.add('BIT', r"0b[0-1]+")
        self.lexer.add('HEX', r"0x[0-9a-fA-F]+")
        # self.lexer.add('REAL', r"([0-9]+(?:\.[0-9]+)?)")
        self.lexer.add('REAL', r"[0-9]+\.[0-9]+")
        self.lexer.add('INTEGER', r"[0-9]+")
        # self.lexer.add('RANGE', r"[0-9]+[\.]{3}[0-9]+")
        # Symbol Identifiers
        self.lexer.add('FUNC_CALL', r"[a-zA-Z][a-zA-Z0-9_]*:")
        self.lexer.add('FUNC_BANG', r"[a-zA-Z]([a-zA-Z0-9_]*)?!:")
        self.lexer.add('FUNC_PRED', r"[a-zA-Z]([a-zA-Z0-9_]*)?\?:")
        self.lexer.add('MATH_CALL', r"[+/*-]{1}:")
        self.lexer.add('MATH_REF', r"[+/*-]{1}")
        # self.lexer.add('LOGICOP_CALL', r'(= | < | <= | > | >= | not=):')
        self.lexer.add('SYMBOL_BANG', r"\b[a-zA-Z]([a-zA-Z0-9_]*)?!")
        self.lexer.add('SYMBOL_PRED', r"\b[a-zA-Z]([a-zA-Z0-9_]*)?\?")
        self.lexer.add('KEYWORD', r":[a-zA-Z]([a-zA-Z0-9_]*)?")
        self.lexer.add('SYMBOL', r"\b[a-zA-Z]([a-zA-Z0-9_]*)?")
        # Ignore comments
        self.lexer.ignore(r";.*?\n")
        # Ignore comma's
        self.lexer.ignore(r',')
        # Ignore spaces
        self.lexer.ignore(r"\s+")

    def get_tokens(self):
        return [x.name for x in self.lexer.rules]

    def get_lexer(self):
        self._add_tokens()
        return self.lexer.build()
