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
from llvmlite import ir, binding
from llvmlite.ir import global_context as glbctx

import ast
from enums import ExpressionType

# from pprint import pformat

LOGGER = logging.getLogger()

int_64 = ir.IntType(64)
int_32 = ir.IntType(32)
void_ptr = ir.PointerType(ir.IntType(8))
any_struct = glbctx.get_identified_type("Any")
any_struct.set_body(int_64, int_64, int_64, int_32, void_ptr)
any_ptr = any_struct.as_pointer()


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

    def _emit_var(self, pvar):
        """Emit a named variable expression"""
        print("{} {}".format(pvar.ptype, pvar.exprs))

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
        self._fn_arg_sigs(pfn)

    def _emit_body(self, ltree):
        if type(ltree) is list:
            if ltree[0].ptype is ExpressionType.MODULE:
                [self._emit_body(n) for n in ltree[0].exprs]
        elif type(ltree) is ast.ParseTree:
            if ltree.ptype is ExpressionType.VARIABLE:
                self._emit_var(ltree)
            elif ltree.ptype is ExpressionType.FUNCTION:
                self._emit_fn(ltree)
        else:
            print("Unknown type")

    def emit(self, ptree, wrtr):
        # Extern declarations
        self._emit_externs(ptree['externs'])
        # Literals
        self._emit_literals(ptree['literals'])
        # Process lambdas
        self._emit_lambda(ptree['lambdas'])
        # Process body
        self._emit_body(ptree['base'])
        # Spit out the goods
        llvm_ir = str(self.module)
        mod = self.binding.parse_assembly(llvm_ir)
        mod.verify()
        self.engine.add_module(mod)
        self.engine.finalize_object()
        self.engine.run_static_constructors()
        wrtr(str(self.module))
