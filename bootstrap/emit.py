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
from functools import singledispatch, update_wrapper
from llvmlite import ir, binding
from llvmlite.ir import global_context as glbctx

import ast
from ptree import *
from enums import ExpressionType

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

    @methdispatch
    def _emit_et(self, el, builder, frame):
        print("Unhandled ParseType {}".format(el.ptype))

    @_emit_et.register(ParseFunction)
    def _emit_func_type(self, el, builder, frame):
        print("In FUNCTION type")

    @_emit_et.register(ParseLambdaRef)
    def _emit_lambdaref_type(self, el, builder, frame):
        print("In LAMBDAREF type")

    @_emit_et.register(ParseLet)
    def _emit_let_type(self, el, builder, frame):
        print("In LET type")

    @_emit_et.register(ParseCall)
    def _emit_call_type(self, el, builder, frame):
        print("In CALL type")

    @_emit_et.register(ParseEmpty)
    def _emit_empty_type(self, el, builder, frame):
        print("In EMPTY type")

    @_emit_et.register(ParseSymbol)
    def _emit_symbol_type(self, el, builder, frame):
        print("In SYMBOL type")

    @_emit_et.register(ParseLiteral)
    def _emit_literal_type(self, el, builder, frame):
        print("In LITERAL type {}", el.name)

    @methdispatch
    def _emit_type(self, el, builder, frame):
        print("Unhandled type {}".format(el))

    @_emit_type.register(ast.FoidlReference)
    def _er(self, el, builder, frame):
        print(" _emit_rtype el {}".format(el))

    @_emit_type.register(ParseTree)
    def _ep(self, el, builder, frame):
        self._emit_et(el, builder, frame)

    def _emit_proc(self, pvar, frame):
        if hasattr(pvar, 'ptype'):
            print(" _emit_proc pvar {}".format(pvar.ptype))
            for p in pvar.exprs:
                self._emit_proc(p, frame)
        else:
            print(" _emti_proc other {}".format(pvar))

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
            self._fn_arg_sigs(ltree)

    def _emit_fn(self, pfn):
        """Emit a Function and it's expressions"""
        fn = self._fn_arg_sigs(pfn)
        builder = ir.IRBuilder(fn.append_basic_block('entry'))
        reta = builder.alloca(any_ptr, name="retcode")
        frame = []
        print("Processing {}".format(pfn.name))
        for t in pfn.exprs:
            self._emit_type(t, builder, frame)
        nila = builder.load(builder.module.get_global("nil"))
        builder.store(nila, reta)
        iret = builder.load(reta)
        builder.ret(iret)
        # print(pfn.exprs)

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
