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
LLLT 	= llvm-ar
CLC 	= clang
LLC     = llc
BSTRP 	= ../bin/fbstrp


# FOIDL Runtime generation settings

FSRC  	:= fsrc/
LLS 	:= ll/
OBJS 	:= objs/
BIN 	:= bin/
HDRS   	:= headers/
RTHDRS  := ../foidlrtl/headers

FDEFS 	:= .defs
FOIDL  	:= .foidl
LL 		:= .ll
OBJ 	:= .o
CSRC	:= .c

foildrt_NAME := foidlrt
foidlrt_LIB  := libfoidlrt.a
STATIC_LIB   := $(LIB)$(foidlrt_LIB)
foidlc_NAME	 := foidlc
FOIDLC		 := $(BIN)$(foidlc_NAME)

# Flags for compilation of all types

BHDRFLAGS := -I $(HDRS) $(RTHDRS) -a hdr
BCFLAGS := -I $(HDRS) $(RTHDRS)
LLCFLAGS = -march=x86-64 -x86-asm-syntax=intel -relocation-model=pic -O3 -filetype=obj

# Substitutions for recipe patterns

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FHRDS   := $(subst $(FSRC), $(HDRS), $(FSRCS:$(FOIDL)=$(FDEFS)))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
FOBJS	:= $(subst $(FSRC), $(OBJS), $(FSRCS:$(FOIDL)=$(OBJ)))

all: $(FOIDLC) | $(FHRDS) $(FLLS)

$(FOIDLC): $(FLLS) $(FHRDS) | $(BIN) $(OBJS) $(LLS)
	$(info Building executable)

$(BIN):
	mkdir bin

headers/%.defs : fsrc/%.foidl
	$(BSTRP) -s $< $(BHDRFLAGS) -o $@

ll/%.ll : fsrc/%.foidl
	$(BSTRP) -s $< $(BCFLAGS) -o $@

$(FLLS):$(FSRCS) | $(FHRDS) $(LLS)

$(LLS):
	mkdir ll

# $(LLC) $< $(LLCFLAGS) -o $@
$(FOBJS):$(FLLS) | $(OBJS)
	$(info making llvm-objs $@)


$(OBJS):
	mkdir objs


clean:
	$(RM) -r $(FHRDS) $(LLS) $(OBJS) $(BIN)
