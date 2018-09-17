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

import enum


@enum.unique
class ParseLevel(enum.Enum):
    FULL = enum.auto()
    LITE = enum.auto()


@enum.unique
class ExpressionType(enum.Enum):
    MODULE = enum.auto()
    FUNCTION = enum.auto()
    VARIABLE = enum.auto()
    COLLECTION = enum.auto()
    EMPTY_COLLECTION = enum.auto()
    LET = enum.auto()
    LET_RES = enum.auto()
    LET_ARG_PAIR = enum.auto()
    MATCH = enum.auto()
    MATCH_RES = enum.auto()
    IF = enum.auto()
    LAMBDA = enum.auto()
    LITERAL = enum.auto()


@enum.unique
class CollTypes(enum.Enum):
    VECTOR = enum.auto()
    LIST = enum.auto()
    MAP = enum.auto()
    SET = enum.auto()
    UNKNOWN = enum.auto()


@enum.unique
class LiteralTypes(enum.Enum):
    CHAR = enum.auto()
    BIT = enum.auto()
    HEX = enum.auto()
    INTEGER = enum.auto()
    REAL = enum.auto()
    STRING = enum.auto()
    KEYWORD = enum.auto()


def literal_dict():
    return dict([(name, {}) for name, _ in LiteralTypes.__members__.items()])


WellKnowns = {
    '+': 'add',
    '-': 'sub',
    '/': 'div',
    '*': 'mul',
    '=': 'eq',
    '<': 'lt',
    '>': 'gt',
    '<=': 'lteq',
    '>=': 'gteq',
    'not=': 'neq'}


# @enum.unique
# class SymbolTypes(enum.Enum):
#     MODULE = enum.auto()
#     VARIABLE = enum.auto()
#     FUNCTION = enum.auto()
#     FUNC_ARG = enum.auto()
#     LAMBDA = enum.auto()
#     LAMBDA_ARGS = enum.auto()
#     LET = enum.auto()
#     LET_RES = enum.auto()
#     LET_ARG = enum.auto()
#     MATCH_RES = enum.auto()
#     EXTERN_FUNC = enum.auto()
#     EXTERN_VAR = enum.auto()
#     UNKNOWN = enum.auto()
