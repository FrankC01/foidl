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


class HLexer():
    def __init__(self):
        self.lexer = LexerGenerator()

    def _add_tokens(self):
        # Keywords
        self.lexer.add('MODULE', r'\bmodule\b')
        # self.lexer.add('INCLUDE', r'include')
        self.lexer.add('VAR', r'\bvar\b')
        self.lexer.add('FUNC', r'\bfunc\b')
        self.lexer.add('PRIVATE', r':private\b')
        # Boolean operators
        # Modifiers
        # self.lexer.add('COMMA', r',')
        # Brackets (symbol lists and lists)
        self.lexer.add('LBRACKET', r'\[')
        self.lexer.add('RBRACKET', r'\]')
        # self.lexer.add('RANGE', r"[0-9]+[\.]{3}[0-9]+")
        # Symbol Identifiers
        self.lexer.add('SYMBOL_BANG', r"[a-zA-Z][a-zA-Z0-9_]+!")
        self.lexer.add('SYMBOL_PRED', r"[a-zA-Z][a-zA-Z0-9_]+\?")
        self.lexer.add('KEYWORD', r":[a-zA-Z]([a-zA-Z0-9_]*)?")
        self.lexer.add('SYMBOL', r"[a-zA-Z]([a-zA-Z0-9_]*)?")
        # Ignores
        self.lexer.ignore(r'\"(.+?)\"')         # String
        self.lexer.ignore(r'\"[\s\S]*?\"')      # String II
        self.lexer.ignore(r'\'(.{1})\'')        # Char
        self.lexer.ignore(r";.*?\n")            # Comment
        self.lexer.ignore(r"\|.*?\n")           # Match Guard
        self.lexer.ignore(r"\d")                # Numbers
        self.lexer.ignore(r"\@")                # Group
        # Character ignores
        self.lexer.ignore(r"[,#%\^\{\}\(\)\>\<\=\?\+\-\*:!]")
        # Ignore spaces
        self.lexer.ignore(r"\s+")

    def get_tokens(self):
        return [x.name for x in self.lexer.rules]

    def get_lexer(self):
        self._add_tokens()
        return self.lexer.build()
