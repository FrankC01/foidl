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
        # Print
        self.lexer.add('PRINT', r"print")
        # Keywords
        self.lexer.add('MODULE', r'module')
        self.lexer.add('FUNC', r'func')
        self.lexer.add('VAR', r'var')
        self.lexer.add('INCLUDE', r'include')
        # Parenthesis
        self.lexer.add('OPEN_PAREN', r"\(")
        self.lexer.add('CLOSE_PAREN', r"\)")
        # Brackets
        self.lexer.add('OPEN_BRACKET', r'\[')
        self.lexer.add('CLOSE_BRACKET', r'\]')
        # Braces
        self.lexer.add('OPEN_BRACES', r'\{')
        self.lexer.add('CLOSE_BRACES', r'\}')
        # Semi Colon
        self.lexer.add('SEMI_COLON', r"\;")
        # Operators
        self.lexer.add('SUM', r"\+")
        self.lexer.add('SUB', r"\-")
        # Number
        self.lexer.add('NUMBER', r"\d+")
        # Ignore spaces
        self.lexer.ignore(r"\s+")

    def get_tokens(self):
        return [x.name for x in self.lexer.rules]

    def get_lexer(self):
        self._add_tokens()
        return self.lexer.build()
