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
# import traceback
from functools import singledispatch, update_wrapper

from llvmlite import ir, binding
from llvmlite.ir import global_context as glbctx

import ast
from ptree import *
# from util import methdispatch
from enums import ExpressionType
from errors import EmitNotImplementedError

# from pprint import pformat

LOGGER = logging.getLogger()


def methdispatch(func):
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


int_64 = ir.IntType(64)
int_32 = ir.IntType(32)
int_8 = ir.IntType(8)
void_ptr = ir.PointerType(ir.IntType(8))
any_struct = glbctx.get_identified_type("Any")
any_struct.set_body(int_64, int_64, int_64, int_32, void_ptr)
any_ptr = any_struct.as_pointer()
any_ptr_ptr = any_ptr.as_pointer()
null_val = ir.FormattedConstant(any_ptr, 'null')

supps = [
    ("foidl_reg_integer", [int_64]),
    ("foidl_reg_keyword", [void_ptr]),
    ("foidl_reg_string", [void_ptr]),
    ("foidl_tofuncref", [void_ptr, any_ptr]),
    ("foidl_imbue", [any_ptr, any_ptr]),
    ("foidl_fref_instance", [any_ptr]),
    ("foidl_truthy_qmark", [any_ptr]),
    ("map_inst_bang", []),
    ("map_extend_bang", [any_ptr, any_ptr, any_ptr]),
    ("list_inst_bang", []),
    ("list_extend_bang", [any_ptr, any_ptr]),
    ("vector_inst_bang", []),
    ("vector_extend_bang", [any_ptr, any_ptr]),
    ("set_inst_bang", []),
    ("set_extend_bang", [any_ptr, any_ptr])]


class LlvmGen(object):
    def __init__(self, source, triple=None):
        # Compilation unit
        self.source = source
        # Setup bindings
        self._binding = binding
        self.binding.initialize()
        self.binding.initialize_native_target()
        self.binding.initialize_native_asmprinter()
        # Setup module
        self._module = ir.Module(self.source)
        self.module.triple = triple if triple \
            else self.binding.get_default_triple()
        target = self.binding.Target.from_triple(self.module.triple)
        target_machine = target.create_target_machine()
        backing_mod = self.binding.parse_assembly("")
        self._engine = self.binding.create_mcjit_compiler(
            backing_mod, target_machine)
        self.vinits = []
        self.fowards = {}

    @property
    def binding(self):
        return self._binding

    @property
    def module(self):
        return self._module

    @property
    def engine(self):
        return self._engine

    def _reg_global_var(self, vname):
        """Create a global variable with null pointer"""
        x = ir.GlobalVariable(self.module, any_ptr, vname)
        x.align = 8
        return x

    def _reg_global_func(self, fname, argcnt):
        """Create a global function with arg count"""
        return ir.Function(
            self.module,
            ir.FunctionType(
                any_ptr,
                [any_ptr] * argcnt if argcnt else []),
            fname)

    def _reg_global_func_wargs(self, fname, args):
        """Create a global function with explicit args"""
        return ir.Function(
            self.module,
            ir.FunctionType(any_ptr, args),
            fname)

    def _reg_global_voidfunc(self, fname, argcnt):
        """Create a global function with arg count"""
        return ir.Function(
            self.module,
            ir.FunctionType(
                ir.VoidType(),
                [any_ptr] * argcnt if argcnt else []),
            fname)

    def _emit_externs(self, extmap):
        """Declare external fn and var references"""
        for k, v in extmap.items():
            if type(v) is ast.VarReference:
                self._reg_global_var(k)
            else:
                self._reg_global_func(k, v.argcnt)
        # Do pre-defines
        [self._reg_global_func_wargs(nm, a) for nm, a in supps]

    def _emit_literals(self, litmap):
        """Declare private literal pointers"""
        for (ok, ov) in litmap.items():
            # If not an empty child map
            if ov:
                for (ik, iv) in ov.items():
                    x = ir.GlobalVariable(self.module, any_ptr, iv.identifier)
                    x.linkage = "private"
                    x.align = 8
                    x.initializer = null_val

    def _register_strtype(self, builder, name, val, strptr, strvptr, fn):
        fs = val.strip('/"') + '\x00'
        sarr = ir.ArrayType(int_8, len(fs))
        bcp = builder.bitcast(strptr, ir.PointerType(sarr))
        builder.store(ir.Constant(sarr, bytearray(fs, 'ascii')), bcp)
        rcall = builder.call(fn, [strvptr])
        targ = builder.module.get_global(name)
        builder.store(rcall, targ)

    def _emit_linits(self, litmap):
        """Emits registration calls for literals of all types"""
        fn = self._reg_global_voidfunc(self.source + "_linits", 0)
        builder = ir.IRBuilder(fn.append_basic_block('entry'))

        clook = {
            "STRING": builder.module.get_global("foidl_reg_string"),
            "KEYWORD": builder.module.get_global("foidl_reg_keyword"),
            "INTEGER": builder.module.get_global("foidl_reg_integer")}

        # Strings and Keywords
        # Get the maximum string/keyword length
        strdict = {**litmap['STRING'], **litmap['KEYWORD']}
        if strdict:
            # Largest buffer
            strptr = builder.alloca(
                ir.ArrayType(
                    int_8,
                    max([len(k) for k in strdict.keys()]) + 1))
            # Base pointer (void *)
            strvptr = builder.bitcast(strptr, void_ptr)
            [[self._register_strtype(
                builder,
                ref.name,
                s,
                strptr,
                strvptr,
                clook[k]) for s, ref in outer_v.items()]
                for k, outer_v in {
                    x: y
                    for x, y in litmap.items()
                    if x in ['KEYWORD', 'STRING']}.items()]

        # Integers
        for k, v in litmap["INTEGER"].items():
            rcall = builder.call(
                clook["INTEGER"],
                [ir.Constant(int_64, int(k))])
            builder.store(rcall, builder.module.get_global(v.name))
        # Close
        builder.ret_void()

    def _emit_vinits(self):
        fn = self._reg_global_voidfunc(self.source + "vinits", 0)
        builder = ir.IRBuilder(fn.append_basic_block('entry'))
        for pvar in self.vinits:
            expr = []
            self._emit_et(pvar.exprs[0], builder, expr)
            builder.store(expr[-1], builder.module.get_global(pvar.name))
        builder.ret_void()

    # Emit expression types

    @methdispatch
    def _emit_et(self, el, builder, frame):
        """Process default when a ParseTree type not handled"""
        if isinstance(el, ParseTree):
            print("Unhandled type {}".format(el.ptype))
        else:
            print("Unhandled type {}".format(el))
        raise EmitNotImplementedError

    @_emit_et.register(ParseLambdaRef)
    def _emit_lambdaref_type(self, el, builder, frame):
        fn = builder.module.get_global(el.name)
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_integer"),
            [ir.Constant(int_64, len(el.exprs))])
        fref = builder.call(
            builder.module.get_global("foidl_tofuncref"),
            [builder.bitcast(fn, void_ptr), mcnt])
        frame.append(fref)

    @_emit_et.register(ParseClosureRef)
    def _emit_closureref_type(self, el, builder, frame):
        # print("ClosureRef {}".format(el))
        fn = builder.module.get_global(el.name)
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_integer"),
            [ir.Constant(int_64, len(el.pre))])
        fref = builder.call(
            builder.module.get_global("foidl_tofuncref"),
            [builder.bitcast(fn, void_ptr), mcnt])
        for i in el.exprs:
            myarg = [fref]
            # print("Closure arg {}".format(i))
            self._emit_et(i, builder, myarg)
            last = builder.call(
                builder.module.get_global("foidl_imbue"),
                myarg)
        frame.append(last)

        # raise EmitNotImplementedError

    @_emit_et.register(ParseLet)
    def _emit_let_type(self, el, builder, frame):
        # Establish return value
        # print("Let res => {}".format(el.res))
        reta = builder.alloca(any_ptr, name=el.res.ident)
        el.res.ptr = reta
        lpair = []
        # Establish letpairs
        # print("Let res => {}".format(el.res))

        for lp in el.pre:
            self._emit_et(lp, builder, lpair)
        # Evaluate body
        expr = []
        for e in el.exprs:
            self._emit_et(e, builder, expr)
        # Set return value
        builder.store(expr[-1], reta)
        # Load return value
        frame.append(builder.load(reta))

    @_emit_et.register(ParseLetPair)
    def _emit_letpair_type(self, el, builder, frame):
        arg = builder.alloca(any_ptr, name=el.res.ident)
        el.res.ptr = arg
        # print("PLP arg =>{}".format(el))
        expr = []
        for e in el.exprs:
            # print("Let Arg Pair {} {}".format(el.res.ident, e))
            self._emit_et(e, builder, expr)
        builder.store(expr[-1], arg)

    @methdispatch
    def _emit_match(self, el, builder, frame, pre=False):
        """Process default when a ParseTree type not handled"""
        if isinstance(el, ParseTree):
            print("MATCH {} Unhandled type {}".format(el, el.ptype))
        else:
            print("MATCH Unhandled type {}".format(el))
        raise EmitNotImplementedError

    @_emit_match.register(ParseMatchPair)
    def _emit_matchpair_type(self, el, builder, frame, pre=None):
        expr = []
        if pre:
            self._emit_et(el.pre[0], builder, expr)
            frame.append(expr[0])
        else:
            self._emit_et(el.exprs[0], builder, expr)
            frame.append(expr[0])

    @_emit_match.register(ParseMatchDefault)
    def _emit_matchdefault_type(self, el, builder, frame, pre=None):
        expr = []
        if pre:
            raise EmitNotImplementedError("Calling default with pre")
        else:
            self._emit_et(el.exprs[0], builder, expr)
            frame.append(expr[0])

    @_emit_match.register(ParseMatchEqual)
    def _emit_matchequal_type(self, el, builder, frame, pre=None):
        foidl_equal = builder.module.get_global("eq")
        expr = []
        if pre:
            self._emit_et(el.pre[0], builder, expr)
            expr.append(builder.load(pre))
            frame.append(builder.call(foidl_equal, expr))
        else:
            print("Equal expr {}".format(el.exprs))
            self._emit_et(el.exprs[0], builder, expr)
            frame.append(expr[0])

    @_emit_et.register(ParseMatch)
    def _emit_match_type(self, el, builder, frame):
        # Establish result and other variables
        bt = builder.load(builder.module.get_global("true"))
        result = builder.alloca(any_ptr, name=el.res[0].ident)  # Result
        expres = builder.alloca(any_ptr, name=el.res[1].ident)  # Expression
        el.res[1].ptr = expres
        guard = builder.alloca(int_64)  # Switch indexer
        # Convenience
        foidl_truthy = builder.module.get_global("foidl_truthy_qmark")
        # Create basic block and jump to it
        matchbb = builder.append_basic_block(el.name)
        builder.branch(matchbb)
        # Establish switch pairs (comparasson and exec expressions)
        with builder.goto_block(matchbb):
            builder.store(ir.Constant(int_64, -1), guard)
            mdefault = el.exprs.pop()     # Remove default from list
            # Process the match expression
            pred = []
            self._emit_et(el.pre[0], builder, pred)
            builder.store(pred[0], expres)
            # First setup the ifs for the match guards derived
            # from each expressions 'pre'
            index = 0
            for p in el.exprs:
                gpred = []
                self._emit_match(p, builder, gpred, expres)
                pr_res = builder.call(foidl_truthy, gpred)
                cmp = builder.icmp_unsigned("==", pr_res, bt)
                with builder.if_then(cmp):
                    builder.store(ir.Constant(int_64, index), guard)
                index += 1

            # Second, setup the switch calls

            sbbs = [builder.append_basic_block(x.name) for x in el.exprs]
            matchdefault = builder.append_basic_block(mdefault.name)
            sw = builder.switch(builder.load(guard), matchdefault)
            index = 0
            for p in sbbs:
                sw.add_case(ir.Constant(int_64, index), p)
                index += 1
            index = 0
            mex = builder.append_basic_block(el.name + "_match_exit")
            # Process normal guard expressions
            for p in el.exprs:
                with builder.goto_block(sbbs[index]):
                    print("{} post pre".format(p))
                    matchexpr = []
                    self._emit_match(
                        el.exprs[index], builder, matchexpr)
                    builder.store(matchexpr[-1], result)
                    builder.branch(mex)
                index += 1
            # Process default guard
            with builder.goto_block(matchdefault):
                matchexpr = []
                self._emit_match(mdefault, builder, matchexpr)
                builder.store(matchexpr[-1], result)
                builder.branch(mex)
            # Then setup the switch pattern
            with builder.goto_block(mex):
                builder.store(bt, result)
                fload = builder.load(result)
                ifex = builder.append_basic_block(el.name + "_exit")
                builder.branch(ifex)
        builder.position_at_end(ifex)
        frame.append(fload)

    @_emit_et.register(ParseIf)
    def _emit_if_type(self, el, builder, frame):
        """Emit If/Then/Else statement"""
        bt = builder.load(builder.module.get_global("true"))
        result = builder.alloca(any_ptr, name=el.res)
        # Setup and branch into if block scope
        ifbb = builder.append_basic_block(el.name)
        builder.branch(ifbb)
        with builder.goto_block(ifbb):
            pr = []
            self._emit_et(el.pre[0], builder, pr)
            pr_res = builder.call(
                builder.module.get_global("foidl_truthy_qmark"),
                pr)
            cmp = builder.icmp_unsigned("==", pr_res, bt)
            # Setup and process then/else constructs
            with builder.if_else(cmp) as (istrue, isfalse):
                with istrue:
                    pt = []
                    self._emit_et(el.exprs[0], builder, pt)
                    builder.store(pt[-1], result)
                with isfalse:
                    pt = []
                    self._emit_et(el.exprs[1], builder, pt)
                    builder.store(pt[-1], result)
            # In endif capture the result
            lload = builder.load(result)
            # Create and branch to exit
            ifex = builder.append_basic_block(el.name + "_exit")
            builder.branch(ifex)
        # Get to last instruction
        builder.position_at_end(ifex)
        frame.append(lload)

    @_emit_et.register(ParseCall)
    def _emit_call_type(self, el, builder, frame):
        fn = builder.module.get_global(el.name)
        args = []
        for a in el.exprs:
            # print("{} expr {}".format(el.name, a))
            self._emit_et(a, builder, args)
        # print(args)
        frame.append(builder.call(fn, args))

    @_emit_et.register(ParsePartialDecl)
    def _emit_partial_decl_type(self, el, builder, frame):
        fn = builder.module.get_global(el.pre.name)
        # Get the full argument count from function
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_integer"),
            [ir.Constant(int_64, len(fn.args))])
        fref = builder.call(
            builder.module.get_global("foidl_tofuncref"),
            [builder.bitcast(fn, void_ptr), mcnt])
        for i in el.exprs:
            myarg = [fref]
            self._emit_et(i, builder, myarg)
            last = builder.call(
                builder.module.get_global("foidl_imbue"),
                myarg)
        frame.append(last)

    @_emit_et.register(ParsePartialInvk)
    def _emit_partial_invk_type(self, el, builder, frame):
        # Get the full argument count from function
        fpre = []
        self._emit_et(el.pre, builder, fpre)
        fref = builder.call(
            builder.module.get_global("foidl_fref_instance"),
            fpre)
        for i in el.exprs:
            myarg = [fref]
            self._emit_et(i, builder, myarg)
            last = builder.call(
                builder.module.get_global("foidl_imbue"),
                myarg)
        frame.append(last)

    @_emit_et.register(ParseGroup)
    def _emit_group(self, el, builder, frame):
        """Build a group of expressions"""
        reta = builder.alloca(any_ptr, name=el.name + "_retcode")
        expr = []
        for i in el.exprs:
            self._emit_et(i, builder, expr)
        builder.store(expr[-1], reta)
        frame.append(builder.load(reta))

    @_emit_et.register(ParseEmpty)
    def _emit_empty_type(self, el, builder, frame):
        self._emit_et(el.exprs[0], builder, frame)

    @_emit_et.register(ast.VarReference)
    def _emit_varref_type(self, el, builder, frame):
        if not el.exprs:
            frame.append(builder.load(builder.module.get_global(el.name)))
        else:
            self._emit_et(el.exprs[0], builder, frame)

    @_emit_et.register(ast.FuncArgReference)
    def _emit_funcargref_type(self, el, builder, frame):
        """Emit function argument reference"""
        # print("FuncArgRef =>".format(el))
        frame.append(builder.function.args[el.argpos])

    @_emit_et.register(ast.LiteralReference)
    def _emit_litref(self, el, builder, frame):
        frame.append(
            builder.load(builder.module.get_global(el.name)))

    @_emit_et.register(ast.LetResReference)
    @_emit_et.register(ast.MatchResReference)
    @_emit_et.register(ast.MatchExprReference)
    def _emit_letresref_type(self, el, builder, frame):
        """Emit function argument reference"""
        frame.append(builder.load(el.ptr))

    @_emit_et.register(ast.LetArgReference)
    def _emit_letargref_type(self, el, builder, frame):
        """Emit function argument reference"""
        # print("LAR => {}".format(el.ptr))
        frame.append(builder.load(el.ptr))

    @_emit_et.register(ast.FuncReference)
    def _emit_funcref_type(self, el, builder, frame):
        """Emit reference to function"""
        fn = builder.module.get_global(el.name)
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_integer"),
            [ir.Constant(int_64, el.argcnt)])
        fref = builder.call(
            builder.module.get_global("foidl_tofuncref"),
            [builder.bitcast(fn, void_ptr), mcnt])
        frame.append(fref)

    def _emit_collection(self, el, builder, ic, ec, cnt=1):
        ires = builder.call(builder.module.get_global(ic), [])
        ecall = builder.module.get_global(ec)
        for ve in zip(*[iter(el.exprs)] * cnt):
            expr = [ires]
            for e in ve:
                self._emit_et(e, builder, expr)
            last = builder.call(ecall, expr)
        return last

    @_emit_et.register(ParseVector)
    def _emit_vector_type(self, el, builder, frame):
        frame.append(self._emit_collection(
            el, builder, "vector_inst_bang", "vector_extend_bang"))

    @_emit_et.register(ParseList)
    def _emit_list_type(self, el, builder, frame):
        frame.append(self._emit_collection(
            el, builder, "list_inst_bang", "list_extend_bang"))

    @_emit_et.register(ParseMap)
    def _emit_map_type(self, el, builder, frame):
        frame.append(self._emit_collection(
            el, builder, "map_inst_bang", "map_extend_bang", 2))

    @_emit_et.register(ParseSet)
    def _emit_set_type(self, el, builder, frame):
        frame.append(self._emit_collection(
            el, builder, "set_inst_bang", "set_extend_bang"))

    @_emit_et.register(ParseSymbol)
    def _emit_symbol_type(self, el, builder, frame):
        # print("Symbol {}".format(el.exprs[0]))
        self._emit_et(el.exprs[0], builder, frame)

    @_emit_et.register(ParseLiteral)
    def _emit_literal_type(self, el, builder, frame):
        frame.append(builder.load(builder.module.get_global(el.exprs[0].name)))

    @methdispatch
    def _emit_type(self, el, builder, frame):
        print("Unhandled type {}".format(el))

    @_emit_type.register(ast.FoidlReference)
    def _er(self, el, builder, frame):
        print(" _emit_rtype el {}".format(el))

    @_emit_type.register(ParseTree)
    def _ep(self, el, builder, frame):
        # print("_ep {}".format(el))
        # print("_ep function {}".format(builder.function))
        self._emit_et(el, builder, frame)

    def _emit_var(self, pvar):
        """Emit a named variable pointer"""
        x = ir.GlobalVariable(self.module, any_ptr, pvar.name)
        x.linkage = "default"
        x.align = 8
        x.initializer = null_val
        # Store for post processing vinits
        self.vinits.append(pvar)

    def _fn_arg_sigs(self, pfn):
        fn = self._reg_global_func(pfn.name, len(pfn.pre))
        # fn = ir.Function(
        #     self.module,
        #     ir.FunctionType(
        #         any_ptr, [any_ptr] * len(pfn.pre)),
        #     pfn.name)
        fn_args = fn.args
        index = 0
        for a in pfn.pre:
            # print("fn arg assign => {}".format(a))
            fn_args[index].name = a.argname
            index += 1
        return fn

    def _emit_fn(self, pfn):
        """Emit a Function and it's expressions"""
        # fn = self._fn_arg_sigs(pfn)
        fn = self.fowards[pfn.name]
        builder = ir.IRBuilder(fn.append_basic_block('entry'))
        reta = builder.alloca(any_ptr, name="retcode")
        # exit_block = fn.append_basic_block("exit")
        frame = []
        for t in pfn.exprs:
            self._emit_type(t, builder, frame)
        # builder.position_at_end(exit_block)
        if frame:
            builder.store(frame[-1], reta)
        iret = builder.load(reta)
        builder.ret(iret)

    def _emit_lambda(self, ltree):
        """Emit a Lamnda and it's expressions"""
        if type(ltree) is list:
            [self._emit_lambda(n) for n in ltree]
        else:
            if not self.fowards.get(ltree.name, None):
                self._emit_frwds(ltree)
            self._emit_fn(ltree)
            # self._fn_arg_sigs(ltree)

    def _emit_frwds(self, pfn):
        self.fowards[pfn.name] = self._fn_arg_sigs(pfn)

    def _emit_body(self, ltree, etype, emeth):
        if type(ltree) is list:
            if ltree[0].ptype is ExpressionType.MODULE:
                [self._emit_body(n, etype, emeth) for n in ltree[0].exprs]
        elif ltree.ptype == etype:
            emeth(ltree)
        else:
            pass

    def emit(self, ptree, wrtr):
        # Extern declarations
        self._emit_externs(ptree['externs'])
        # Literals
        self._emit_literals(ptree['literals'])
        # Process body variables
        self._emit_body(ptree['base'], ExpressionType.VARIABLE, self._emit_var)
        # Forward decls functions
        self._emit_body(
            ptree['base'], ExpressionType.FUNCTION, self._emit_frwds)
        # Process lambdas
        self._emit_lambda(ptree['lambdas'])
        # Process body
        self._emit_body(ptree['base'], ExpressionType.FUNCTION, self._emit_fn)
        # Generate the literal initializers
        self._emit_linits(ptree['literals'])
        # Generate the variable initializers
        self._emit_vinits()
        # Spit out the goods
        llvm_ir = str(self.module)
        mod = self.binding.parse_assembly(llvm_ir)
        mod.verify()
        self.engine.add_module(mod)
        # self.engine.finalize_object()
        # self.engine.run_static_constructors()
        wrtr(str(self.module))
