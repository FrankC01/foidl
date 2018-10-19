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


class FoidlTokenType(object):
    def __init__(self, token):
        self._token = token

    @property
    def token(self):
        return self._token

    def gettokentype(self):
        return self.token.gettokentype()

    def getsourcepos(self):
        return self.token.getsourcepos()

    def getstr(self):
        return self.token.getstr()

    def __repr__(self):
        return "{}({}, {})".format(
            type(self).__name__,
            self.gettokentype(), self.getstr())


class MODULE(FoidlTokenType): pass
class INCLUDE(FoidlTokenType): pass
class VAR(FoidlTokenType): pass
class FUNC(FoidlTokenType): pass
class LAMBDA(FoidlTokenType): pass

class FUNC_CALL(FoidlTokenType): pass
class FUNC_BANG(FoidlTokenType): pass
class FUNC_PRED(FoidlTokenType): pass
class MATH_CALL(FoidlTokenType): pass
class LT_CALL(FoidlTokenType): pass
class GT_CALL(FoidlTokenType): pass
class LTEQ_CALL(FoidlTokenType): pass
class GTEQ_CALL(FoidlTokenType): pass
class NOTEQ_CALL(FoidlTokenType): pass

class LPAREN(FoidlTokenType): pass
class RPAREN(FoidlTokenType): pass
class LANGLE(FoidlTokenType): pass
class RANGLE(FoidlTokenType): pass
class LBRACKET(FoidlTokenType): pass
class RBRACKET(FoidlTokenType): pass
class LBRACE(FoidlTokenType): pass
class RBRACE(FoidlTokenType): pass
class LSET(FoidlTokenType): pass

class LET(FoidlTokenType): pass

class GROUP(FoidlTokenType): pass

class IF(FoidlTokenType): pass

class MATCH(FoidlTokenType): pass
class MATCH_PATTERN(FoidlTokenType): pass
class MATCH_EXPRREF(FoidlTokenType): pass
class MATCH_DEFAULT(FoidlTokenType): pass


class STRING(FoidlTokenType): pass
class CHAR(FoidlTokenType): pass
class BIT(FoidlTokenType): pass
class HEX(FoidlTokenType): pass
class REAL(FoidlTokenType): pass
class INTEGER(FoidlTokenType): pass
class KEYWORD(FoidlTokenType): pass
class EQ_CALL(FoidlTokenType): pass

class IF_REF(FoidlTokenType): pass
class EQ_REF(FoidlTokenType): pass
class LT_REF(FoidlTokenType): pass
class GT_REF(FoidlTokenType): pass
class LTEQ_REF(FoidlTokenType): pass
class GTEQ_REF(FoidlTokenType): pass
class NOTEQ_REF(FoidlTokenType): pass

class SYMBOL(FoidlTokenType): pass
class SYMBOL_BANG(FoidlTokenType): pass
class SYMBOL_PRED(FoidlTokenType): pass

class COMMA(FoidlTokenType): pass


def foidl_tfactory(cname, token):
    if not globals().get(cname, None):
        subClass = type(cname, (FoidlTokenType,), {})
        globals()[cname] = subClass
    else:
        subClass = globals()[cname]
    return subClass(token)


class FoidlTokenStream(object):
    def __init__(self, tokens):
        self._tokens = list(reversed(
            [foidl_tfactory(x.gettokentype(), x) for x in tokens]))
        self._count = len(self._tokens)
        self._active = self._tokens
        self._current = 0

    @property
    def tokens(self):
        return self._active

    @property
    def count(self):
        return self._count

    @property
    def current(self):
        return self._current

    def incpos(self):
        self._current += 1

    def dump(self):
        while True:
            if self.current < self.count:
                print(self.tokens[self.current])
                self.incpos()
            else:
                break

    @property
    def ismore(self):
        return self.current < self.count

    def isnext(self, ttype):
        if self.ismore:
            return True if type(self.tokens[self.current]) is ttype else False
        else:
            return False

    def pushback(self):
        self.__current -= 1

    def getnext(self, etype=None):
        if self.ismore:
            if etype:
                if self.isnext(etype):
                    return next(self)
                else:
                    raise IOError
            else:
                return next(self)
        else:
            return None

    def _inext(self):
        t = self.tokens[self.current]
        self.incpos()
        return t

    def __next__(self):
        return self._inext()

    def __iter__(self):
        while True:
            if self.ismore:
                yield next(self)
            else:
                break
