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

import enums
from abc import ABC, abstractmethod


class SymbolException(Exception):
    pass


class BaseReference(ABC):
    """BaseReference informs type, value, intern/extern, etc"""

    def __init__(self, token):
        self._token = token

    @property
    def token(self):
        return self._token


class LiteralReference(BaseReference):
    """Literal value reference"""
    pass


class VarReference(BaseReference):
    """Variable Symbol Reference"""
    pass


class FuncArgReference(BaseReference):
    """Function Argument Symbol Reference"""
    pass


class FuncReference(BaseReference):
    """FuncReference informs name, intern/extern, arg-count"""
    pass


class LambdaReference(BaseReference):
    """Lambda Reference"""
    pass


class LetArgReference(BaseReference):
    """Let Argument Symbol Reference"""
    pass


class LetResReference(BaseReference):
    """Let Result Symbol Reference"""
    pass


class MatchResReference(BaseReference):
    """Match Result Symbol Reference"""
    pass


class SymbolTable():
    """SymbolTable exists for each complex type

    This includes module, var, func, func_args, let_res, let_args, lambdas)
    """
    pass


class SymbolTree():
    """Manages the tree of symbols and is preserved until emit"""
    pass
