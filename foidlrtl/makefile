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

CC      = g++
LT      = libtool
LLC     = llc
CLC 	= clang
BSTRP 	= ../bin/fbstrp


# FOIDL Runtime generation settings

FSRC  	= fsrc/
SRC  	= src/
INC 	= includes/
LLS 	= ll/
OBJ 	= obj/
LIB 	= lib/
HDRS   	= headers/

FHRDS 	= .defs
FOIDLS  = .foidl


CLFLAG   = -g -Wall -c -I$(G2CINC)
LLFLAGS  = -L $(LIBS) -mllvm --x86-asm-syntax=intel -l$(foildrt_NAME) -Wall -g -L. -Wl,-pie -I. -I$(INCS)
G2LLFLAGS  = -L $(G2LIBS) -mllvm --x86-asm-syntax=intel -l$(foildrt2_NAME) -Wall -g -L. -Wl,-pie -I. -I$(INCS)

LLCFLAGS = -march=x86-64 -x86-asm-syntax=intel -relocation-model=pic
LLALAGS  = -c -mllvm --x86-asm-syntax=intel -Wall -g -I. -I$(INCS)

# all: $(STATIC_LIB)

# $(STATIC2_LIB): $(COREH) $(G2COREMH) $(G2COREASM) $(G2COREMASM) $(COBJS)
# 	$(LT) -static -s -o $(STATIC2_LIB) $(ASMOBJS) $(G2COREASM) $(G2COREMASM) $(COBJS)

# Invocation wrapper

$(HDRS)langcore$(FHRDS): $(FSRC)langcore$(FOIDLS)
	$(BSTRP) -s $< -I $(HDRS) -r -a hdr -o $@

$(HDRS)langcorem$(FHRDS): $(FSRC)langcorem$(FOIDLS)
	$(BSTRP) -s $< -I $(HDRS) -r -a hdr -o $@

#clean:
#	$(RM) $(ASMOBJS) $(AGLLOBJS) $(G2COREASM) $(FOC2S) $(G2COREH) $(G2CORELL) $(G2COREMASM) $(FOC2MS) $(G2COREMH) $(G2COREMLL) $(STATIC2_LIB)
