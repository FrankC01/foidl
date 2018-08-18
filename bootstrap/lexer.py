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
        self.lexer.add('MODULE', r'module')
        self.lexer.add('INCLUDE', r'include')
        self.lexer.add('VAR', r'var')
        self.lexer.add('FUNC', r'func')
        self.lexer.add('LET', r'let')
        self.lexer.add('LAMBDA', r"\^")
        self.lexer.add('NIL', r'nil')
        self.lexer.add('MATCH', r'match')
        self.lexer.add('MATCH_PATTERN', r'\|')
        self.lexer.add('GROUP', r'@\(')
        self.lexer.add('MAIN', r'main')
        self.lexer.add('IF', r'\?:')
        self.lexer.add('IF_REF', r'\?')
        # Sugar for built-ins
        self.lexer.add('ADD_CALL', r"\+:")
        self.lexer.add('ADD_REF', r"\+")
        self.lexer.add('SUB_CALL', r"-:")
        self.lexer.add('SUB_REF', r"-")
        self.lexer.add('MUL_CALL', r"\*:")
        self.lexer.add('MUL_REF', r"\*")
        self.lexer.add('DIV_CALL', r"/:")
        self.lexer.add('DIV_REF', r"/")
        # Modifiers
        self.lexer.add('MOD', r'%')
        self.lexer.add('AMP', r'&')
        self.lexer.add('COMMA', r',')
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
        self.lexer.add('STRING', r'\"(.+?)\"')
        self.lexer.add('CHAR', r'\'(.{1})\'')
        # Numbers
        self.lexer.add('BIT', r"0b[0-9a-fA-F]+")
        self.lexer.add('HEX', r"0x[0-9a-fA-F]+")
        # self.lexer.add('REAL', r"([0-9]+(?:\.[0-9]+)?)")
        self.lexer.add('REAL', r"[0-9]+\.[0-9]+")
        self.lexer.add('INTEGER', r"[0-9]+")
        # self.lexer.add('RANGE', r"[0-9]+[\.]{3}[0-9]+")
        # Identifiers
        self.lexer.add('FUNC_CALL', r"[a-zA-Z][a-zA-Z0-9_]+:")
        self.lexer.add('SYMBOL_BANG', r"[a-zA-Z][a-zA-Z0-9_]+!")
        self.lexer.add('KEYWORD', r":[a-zA-Z]([a-zA-Z0-9_]*)?")
        self.lexer.add('SYMBOL', r"[a-zA-Z]([a-zA-Z0-9_]*)?")
        # Ignore comments
        self.lexer.ignore(r";.*?\n")
        # Ignore spaces
        self.lexer.ignore(r"\s+")

    def get_tokens(self):
        return [x.name for x in self.lexer.rules]

    def get_lexer(self):
        self._add_tokens()
        return self.lexer.build()
