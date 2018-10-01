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

import logging
import copy
from enums import ExpressionType
from ptree import ParseSymbol, ParseLambda, ParseLambdaRef, ParseClosureRef


LOGGER = logging.getLogger()


def _find_refs(argsig, srcpos, body, cntrl, clist, newlist, bexpr=None):
    """Find closures candidates in body.

    These are identifiable as some reference types
    that have a token source position elsewhere
    said references"""

    def _inner(el, elref, elpos, bexpr):
        if elref in argsig:
            return
        elif elpos.lineno > srcpos.lineno:
            return
        elif elpos.lineno < srcpos.lineno or elpos.colno < srcpos.colno:
            index = bexpr.index(el)
            nc = copy.deepcopy(el)
            nc.exprs[0] = cntrl[0].replicate(elref)
            bexpr[index] = nc
            newlist.append(nc)
            clist.append(el)
        else:
            print("     UNKNOWN")

    if type(body) is list:
        [_find_refs(
            argsig, srcpos, n, cntrl, clist, newlist) for n in body]
    elif type(body) is ParseSymbol and type(body.exprs[0]) in cntrl:
        _inner(
            body, body.exprs[0], body.exprs[0].token.getsourcepos(), bexpr)
    else:
        [_find_refs(
            argsig, srcpos, n, cntrl, clist, newlist, body.exprs)
            for n in body.exprs]


def refactor_lambda(astref, srcpos, argsig, body, cntrl):
    """Update lambda so it captures free variables"""
    clist = []
    newlist = []
    _find_refs(argsig, srcpos, body, cntrl, clist, newlist)
    if clist:
        # Get the argsig references, extend, reindex
        newfa = [n.exprs[0] for n in newlist]
        newfa.extend(argsig)
        index = 0
        for n in newfa:
            n.argpos = index
            index += 1

        # [print("Clist {}".format(n.exprs[0])) for n in clist]
        # [print("Newlist {}".format(n.exprs[0])) for n in newlist]

        return [
            ParseLambda(
                ExpressionType.LAMBDA,
                body,
                astref.token,
                astref.name.value,
                pre=newfa),
            ParseClosureRef(
                ExpressionType.CLOSURE_REF,
                clist,
                astref.token,
                astref.name.value,
                pre=newfa)]
    else:
        return [
            ParseLambda(
                ExpressionType.LAMBDA,
                body,
                astref.token,
                astref.name.value,
                pre=argsig),
            ParseLambdaRef(
                ExpressionType.LAMBDA_REF,
                argsig,
                astref.token,
                astref.name.value)]
