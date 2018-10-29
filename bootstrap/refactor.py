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

from errors import ParseError
from enums import ExpressionType
from ptree import ParseSymbol, ParseLambda, ParseLambdaRef, ParseClosureRef


LOGGER = logging.getLogger()


def _find_refs(
        argsig, srcpos, body, cntrl, ecntrl, clist, newlist, bexpr=None):
    """Find closures candidates in body.

    These are identifiable as some reference types
    that have a token source position prior to lambda declaration
    """

    def _deep_inner(el, elref, elpos, bexpr):
        # If the element does not preceed lambda position or
        # is not already in the lambdas function arguments
        if elref in argsig or elpos.lineno > srcpos.lineno:
            return
        # If the element  does proceed the lambda expression
        elif elpos.lineno < srcpos.lineno or elpos.colno < srcpos.colno:
            # Get the index of the element from element expression list
            index = bexpr.index(el)
            # Make a copy of the element
            nc = copy.deepcopy(el)
            # Replicate the expression to FuncArgReference
            nc.exprs[0] = cntrl[0].replicate(elref)
            # Replace base expression with funcarg reference
            bexpr[index] = nc
            # Copy to function arg newlist to indicate it is new
            newlist.append(nc)
            # Add to closures list
            clist.append(el)
        else:
            raise ParseError(
                "Unhandled lambda free variable location {}".format(
                    elpos))

    def _lite_inner(el, elref, elpos, bexpr):
        # If the element does not preceed lambda position or
        # is not already in the lambdas function arguments
        if elref in argsig or elpos.lineno > srcpos.lineno:
            return
        # If the element  does proceed the lambda expression
        elif elpos.lineno < srcpos.lineno or elpos.colno < srcpos.colno:
            # Get the index of the element from element expression list
            index = bexpr.index(el)
            # Make a copy of the element
            # nc = copy.deepcopy(el)
            # Replicate the expression to FuncArgReference
            # To make this work we need to create a ParseSymbol
            # for the lite-weights (MatchExpr result, etc)
            nc = cntrl[0].replicate(elref)
            nc.name = el.ident
            nc.ident = el.ident
            psymbol = ParseSymbol(
                ExpressionType.SYMBOL_REF,
                [nc],
                elref.token,
                el.ident)
            # nc.exprs[0] = cntrl[0].replicate(elref)
            # Replace base expression with funcarg reference
            # bexpr[index] = nc
            bexpr[index] = psymbol
            # Copy to function arg newlist to indicate it is new
            # newlist.append(nc)
            newlist.append(psymbol)
            # Add to closures list
            clist.append(el)
        else:
            raise ParseError(
                "Unhandled lambda free variable location {}".format(
                    elpos))

    if type(body) is list:
        [_find_refs(
            argsig, srcpos, n, cntrl, ecntrl, clist, newlist) for n in body]
    # If type is in ecntrl ignore list
    elif type(body) in ecntrl:
        pass
    # If Symbol and it's expression in control list
    elif type(body) is ParseSymbol and type(body.exprs[0]) in cntrl:
        _deep_inner(
            body, body.exprs[0], body.exprs[0].token.getsourcepos(), bexpr)
    # If not symbol, then probably a reference of lite weight
    elif not hasattr(body, 'exprs') and type(body) in cntrl:
        _lite_inner(
            body, body, body.token.getsourcepos(), bexpr)
    # Otherwise, assume it is a Deeper type
    else:
        [_find_refs(
            argsig, srcpos, n, cntrl, ecntrl, clist, newlist, body.exprs)
            for n in body.exprs]


def refactor_lambda(astref, srcpos, argsig, body, cntrl, ecntrl):
    """Update lambda so it captures free variables"""
    clist = []      # Definition argument signature
    newlist = []    # New list is new item list
    _find_refs(argsig, srcpos, body, cntrl, ecntrl, clist, newlist)
    if clist:
        # Get the argsig references from newlist
        newfa = [n.exprs[0] for n in newlist]
        # Extends new ones with existing
        newfa.extend(argsig)
        # Reindex the lambda signature
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
                True,
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
                True,
                pre=argsig),
            ParseLambdaRef(
                ExpressionType.LAMBDA_REF,
                argsig,
                astref.token,
                astref.name.value)]
