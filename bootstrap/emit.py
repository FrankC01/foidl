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
import traceback
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
void_ptr = ir.PointerType(ir.IntType(8))
any_struct = glbctx.get_identified_type("Any")
any_struct.set_body(int_64, int_64, int_64, int_32, void_ptr)
any_ptr = any_struct.as_pointer()
any_ptr_ptr = any_ptr.as_pointer()
null_val = ir.FormattedConstant(any_ptr, 'null')

supps = [
    ("foidl_reg_int", [int_64]),
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

    @property
    def binding(self):
        return self._binding

    @property
    def module(self):
        return self._module

    @property
    def engine(self):
        return self._engine

    def _emit_externs(self, extmap):
        """Declare external fn and var references"""
        for k, v in extmap.items():
            if type(v) is ast.VarReference:
                x = ir.GlobalVariable(self.module, any_ptr, k)
                x.align = 8
            else:
                ir.Function(
                    self.module,
                    ir.FunctionType(
                        any_ptr,
                        [any_ptr] * v.argcnt if v.argcnt else []),
                    k)
        # Do pre-defines

        def suppl_gen(name, args):
            ir.Function(
                self.module,
                ir.FunctionType(any_ptr, args),
                name)
        [suppl_gen(nm, a) for nm, a in supps]

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
        print("In LAMBDAREF type {} {}".format(el.name, el.exprs))
        fn = builder.module.get_global(el.name)
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_int"),
            [ir.Constant(int_64, len(el.exprs))])
        fref = builder.call(
            builder.module.get_global("foidl_tofuncref"),
            [builder.bitcast(fn, void_ptr), mcnt])
        frame.append(fref)
        # raise EmitNotImplementedError

    @_emit_et.register(ParseLet)
    def _emit_let_type(self, el, builder, frame):
        # Establish return value
        reta = builder.alloca(any_ptr, name=el.res.ident)
        el.res.ptr = reta
        lpair = []
        # Establish letpairs
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
        expr = []
        for e in el.exprs:
            self._emit_et(e, builder, expr)
        builder.store(expr[-1], arg)

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
            self._emit_et(a, builder, args)
        # print(args)
        frame.append(builder.call(fn, args))

    @_emit_et.register(ParsePartialDecl)
    def _emit_partial_decl_type(self, el, builder, frame):
        fn = builder.module.get_global(el.pre.name)
        # Get the full argument count from function
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_int"),
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
        frame.append(builder.function.args[el.argpos])

    @_emit_et.register(ast.LetResReference)
    def _emit_letresref_type(self, el, builder, frame):
        """Emit function argument reference"""
        # traceback.print_stack()
        frame.append(builder.load(el.ptr))

    @_emit_et.register(ast.LetArgReference)
    def _emit_letargref_type(self, el, builder, frame):
        """Emit function argument reference"""
        frame.append(builder.load(el.ptr))

    @_emit_et.register(ast.FuncReference)
    def _emit_funcref_type(self, el, builder, frame):
        """Emit reference to function"""
        fn = builder.module.get_global(el.name)
        mcnt = builder.call(
            builder.module.get_global("foidl_reg_int"),
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
        self._emit_et(el, builder, frame)

    def _emit_var(self, pvar):
        """Emit a named variable expression"""
        # print("{} {}".format(pvar.ptype, pvar.exprs))
        x = ir.GlobalVariable(self.module, any_ptr, pvar.name)
        x.linkage = "default"
        x.align = 8
        x.initializer = null_val
        self.vinits.append(pvar)
        # print(frame)

    def _fn_arg_sigs(self, pfn):
        fn = ir.Function(
            self.module,
            ir.FunctionType(
                any_ptr, [any_ptr] * len(pfn.pre)),
            pfn.name)
        fn_args = fn.args
        index = 0
        for a in pfn.pre:
            fn_args[index].name = a.argname
            index += 1
        return fn

    def _emit_lambda(self, ltree):
        """Emit a Lamnda and it's expressions"""
        if type(ltree) is list:
            [self._emit_lambda(n) for n in ltree]
        else:
            self._emit_fn(ltree)
            # self._fn_arg_sigs(ltree)

    def _emit_fn(self, pfn):
        """Emit a Function and it's expressions"""
        fn = self._fn_arg_sigs(pfn)
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

    def _emit_body(self, ltree, etype, emeth):
        if type(ltree) is list:
            if ltree[0].ptype is ExpressionType.MODULE:
                [self._emit_body(n, etype, emeth) for n in ltree[0].exprs]
        elif ltree.ptype == etype:
            emeth(ltree)
        else:
            pass
            # print("Unknown type")

    def emit(self, ptree, wrtr):
        # Extern declarations
        self._emit_externs(ptree['externs'])
        # Literals
        self._emit_literals(ptree['literals'])
        # Process body variables
        self._emit_body(ptree['base'], ExpressionType.VARIABLE, self._emit_var)
        # Process lambdas
        self._emit_lambda(ptree['lambdas'])
        # Process body
        self._emit_body(ptree['base'], ExpressionType.FUNCTION, self._emit_fn)
        # Spit out the goods
        llvm_ir = str(self.module)
        mod = self.binding.parse_assembly(llvm_ir)
        mod.verify()
        self.engine.add_module(mod)
        # self.engine.finalize_object()
        # self.engine.run_static_constructors()
        wrtr(str(self.module))
