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

LT      = libtool
LLLT 	= llvm-ar
CLC 	= clang
BSTRP 	= python ../bin/fbstrp


# Get OS information

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	OS_NAME := $(UNAME)
else ifeq ($(UNAME), Solaris)
	OS_NAME := $(UNAME)
else ifeq ($(UNAME), Darwin)
	OS_NAME := $(UNAME)
else
	OS_NAME := Windows
endif



# FOIDL Runtime generation settings

FSRC  	:= fsrc/
SRC  	:= src/
INC 	:= includes/
LLS 	:= ll/
OBJS 	:= objs/
LIB 	:= lib/
HDRS   	:= headers/

FDEFS 	:= .defs
FOIDL  	:= .foidl
LL 		:= .ll
OBJ 	:= .o
CSRC	:= .c

foildrt_NAME := foidlrt

# Flags for compilation of all types

BHDRFLAGS := -I $(HDRS) -r -a hdr
BCFLAGS := -I $(HDRS) -r


# Substitutions for recipe patterns

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FHRDS   := $(subst $(FSRC), $(HDRS), $(FSRCS:$(FOIDL)=$(FDEFS)))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
FOBJS	:= $(subst $(FSRC), $(OBJS), $(FSRCS:$(FOIDL)=$(OBJ)))

SRCS	:= $(wildcard $(SRC)*$(CSRC))
SOBJS	:= $(subst $(SRC), $(OBJS), $(SRCS:$(CSRC)=$(OBJ)))

ifeq ($(OS_NAME), Windows)
	CLLFLAGS := -I $(INC) -march=x86-64 -std=c11 -O3 -c
	foidlrt_LIB  := foidlrt.lib
	STATIC_LIB   := $(LIB)$(foidlrt_LIB)
	LIBFLAGS := -ru 
else
	CLLFLAGS := -I $(INC) -march=x86-64 -fPIC -O3 -c
	foidlrt_LIB  := libfoidlrt.a
	STATIC_LIB   := $(LIB)$(foidlrt_LIB)
	LIBFLAGS :=  --format=default -ru 
endif

all: $(STATIC_LIB) | $(FLLS) $(OBJS)

$(STATIC_LIB): $(FOBJS) $(SOBJS) | $(LIB)
	$(LLLT) $(LIBFLAGS) $(STATIC_LIB) $(SOBJS)

$(LIB):
	mkdir lib

# FOIDL Source -> headers and -> ll

headers/%.defs : fsrc/%.foidl
	$(BSTRP) -s $< $(BHDRFLAGS) -o $@

ll/%.ll : fsrc/%.foidl
	$(BSTRP) -s $< $(BCFLAGS) -o $@

$(FLLS):$(FSRCS) | $(FHRDS) $(LLS)

$(LLS):
	mkdir ll

objs/%.o : ll/%.ll
	$(CLC) $< $(CLLFLAGS) -o $@

$(FOBJS):$(FLLS) | $(OBJS)

# C file compilations

objs/%.o : src/%.c
	$(CLC) $< $(CLLFLAGS) -o $@

$(SOBJS):$(SRCS) | $(OBJS)

$(OBJS):
	mkdir objs


clean:
	$(RM) -r $(FHRDS) $(LLS) $(OBJS) $(LIB)
