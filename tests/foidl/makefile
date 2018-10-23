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
BSTRP 	= python ../../bin/fbstrp

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
LLS 	:= ll/
BIN 	:= bin/
RTHDRS  := ../../foidlrtl/headers
RTLIB 	:= ../../foidlrtl/lib

FOIDL  	:= .foidl
EXE 	:= .exe
LL 		:= .ll

foidlrt_NAME := foidlrt
foidlrt_LIB  := libfoidlrt.a

# Flags for compilation of all types

BCFLAGS := -I $(RTHDRS)
CEXEFLAGS = -l$(foidlrt_NAME) -L $(RTLIB) -m64

# Substitutions for recipe patterns

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
# Need some kinda windows switch here
FEXES	:= $(basename $(subst $(FSRC), $(BIN), $(FSRCS:$(FOIDL)=$(EXE))))

all: $(FEXES) | $(BIN) $(LLS)

bin/%: ll/%.ll
	$(CLC) $< $(CEXEFLAGS)  -o $@

$(FEXES): $(FLLS) | $(BIN)

$(BIN):
	mkdir bin

ll/%.ll : fsrc/%.foidl
	$(BSTRP) -s $< $(BCFLAGS) -o $@

$(FLLS):$(FSRCS) | $(LLS)

$(LLS):
	mkdir ll

clean:
	$(RM) -r $(LLS) $(BIN)
