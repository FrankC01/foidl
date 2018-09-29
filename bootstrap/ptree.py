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


from rply import Token


class ParseTree(object):

    def __init__(self, ptype, exprs, loc=None, name=None, res=None, pre=None):
        self._ptype = ptype
        self._exprs = exprs
        self._loc = loc if type(loc) is not Token else loc.getsourcepos()
        self._name = name if name else "unknown"
        self._res = res
        self._pre = pre if pre else []

    @property
    def ptype(self):
        return self._ptype

    @property
    def exprs(self):
        return self._exprs

    @property
    def loc(self):
        return self._loc

    @property
    def name(self):
        return self._name

    @property
    def res(self):
        return self._res

    @property
    def pre(self):
        return self._pre


class ParseFunction(ParseTree):
    pass


class ParseCall(ParseTree):
    pass


class ParseLambda(ParseTree):
    pass


class ParsePartialDecl(ParseTree):
    pass


class ParseGroup(ParseTree):
    pass


class ParsePartialInvk(ParseTree):
    pass


class ParseLambdaRef(ParseTree):
    pass


class ParseLet(ParseTree):
    pass


class ParseLetPair(ParseTree):
    pass


class ParseEmpty(ParseTree):
    pass


class ParseLiteral(ParseTree):
    pass


class ParseSymbol(ParseTree):
    pass
