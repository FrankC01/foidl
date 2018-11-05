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

# llvm toolchain executables

CLC 	:= clang
CLINK   := lld-link

FSRC  	:= fsrc/
LLS 	:= ll/
OBJS 	:= objs/
BIN 	:= bin/
HDRS   	:= headers/
RTHDRS  := ../foidlrtl/headers
RTLIB 	:= ../foidlrtl/lib

# FOIDL Runtime generation settings

FDEFS 	:= .defs
FOIDL  	:= .foidl
LL 		:= .ll
OBJ 	:= .o

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FHDRS   := $(subst $(FSRC), $(HDRS), $(FSRCS:$(FOIDL)=$(FDEFS)))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
FOBJS	:= $(subst $(FSRC), $(OBJS), $(FSRCS:$(FOIDL)=$(OBJ)))

foildrt_NAME := foidlrt
foidlc_NAME	 := foidlc
# Need some kinda windows switch here
ifeq ($(OS_NAME), Windows)
	foidlrt_LIB  := libfoidlrt.lib
	FOIDLC		 := $(BIN)$(foidlc_NAME).exe
else
	foidlrt_LIB  := libfoidlrt.a
	FOIDLC		 := $(BIN)$(foidlc_NAME)
endif
STATIC_LIB   := $(LIB)$(foidlrt_LIB)

# Flags for compilation of all types
ifeq ($(OS_NAME), Windows)
	ARCH32 := x86
	ARCH64 := x64
	ARCH := $(ARCH64) # set to either one, and the right stuff will get chosen
	DEFINES := -DWIN32 -std=c++11 #-fno-rtti -fno-exceptions -D_HAS_EXCEPTIONS=0 -D_ITERATOR_DEBUG_LEVEL=0
	_MSC_VER := 1900 # 1800=VC2013, 1900=VC2015, 1910=VC2017 / others: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
	LIBS := libcmt.lib libcpmt.lib $(foidlrt_LIB)

	# base locations of MSVC toolchain:
	# search location #1
	PROGRAMFILES_PATH1=$(HOME)/MSVC
	# search location #2
	PROGRAMFILES_PATH2=C:\Program Files (x86)
	PROGRAMFILES_PATH=$(shell if [ -d "$(PROGRAMFILES_PATH1)" ]; then echo "$(PROGRAMFILES_PATH1)"; else echo "$(PROGRAMFILES_PATH2)"; fi )
	WINDOWS_KITS10_PATH=$(PROGRAMFILES_PATH)/Windows Kits/10
	WINDOWS_KITS81_PATH=$(PROGRAMFILES_PATH)/Windows Kits/8.1
	MSVC_PATH=$(PROGRAMFILES_PATH)/Microsoft Visual Studio 14.0/VC
	# Some universal flags
	UniversalCRT_IncludePath64="$(WINDOWS_KITS10_PATH)/Include/10.0.10150.0/ucrt"
	UniversalCRT_Lib64="$(WINDOWS_KITS10_PATH)/Lib/10.0.10150.0/ucrt/$(ARCH64)"
	MSVC_INCLUDE64="$(MSVC_PATH)/include"
	MSVC_LIB64="$(MSVC_PATH)/lib$(if $(filter $(ARCH64),x64),/amd64,)"
	WINSDK_INC64="$(WINDOWS_KITS81_PATH)/Include/um"
	WINSDK_SHARED_INC64="$(WINDOWS_KITS81_PATH)/Include/shared"
	WINSDK_LIB64="$(WINDOWS_KITS81_PATH)/Lib/winv6.3/um/$(ARCH64)"

	TARGET=-target $(if $(filter $(ARCH),x64),x86_64-pc-windows-msvc,i386-pc-win32)
	TARGET_64=-target x86_64-pc-windows-msvc -m64
	TARGETCL=/arch:$(if $(filter $(ARCH),x64),-m64,-m32)
	TARGETCL_64=-m64 /arch:AVX2

	SYSTEM_INC=-isystem $(MSVC_INCLUDE) -isystem $(UniversalCRT_IncludePath) -isystem $(WINSDK_INC) -isystem $(WINSDK_SHARED_INC)
	SYSTEM_INC_64=-isystem $(MSVC_INCLUDE64) -isystem $(UniversalCRT_IncludePath64) -isystem $(WINSDK_INC64) -isystem $(WINSDK_SHARED_INC64)
	SYSTEMCL_INC=/imsvc $(MSVC_INCLUDE) /imsvc $(UniversalCRT_IncludePath) /imsvc $(WINSDK_INC) /imsvc $(WINSDK_SHARED_INC)
	SYSTEMCL_INC_64=/imsvc $(MSVC_INCLUDE64) /imsvc $(UniversalCRT_IncludePath64) /imsvc $(WINSDK_INC64) /imsvc $(WINSDK_SHARED_INC64)
	SYSTEM_LIB_INC=/libpath:$(MSVC_LIB) /libpath:$(UniversalCRT_Lib) /libpath:$(WINSDK_LIB)
	SYSTEM_LIB_INC_64=/libpath:$(MSVC_LIB64) /libpath:$(UniversalCRT_Lib64) /libpath:$(WINSDK_LIB64)
	SYSTEM_LIBGCC_INC=-L$(MSVC_LIB) -L$(UniversalCRT_Lib) -L$(WINSDK_LIB)
	SYSTEM_LIBGCC_INC_64=-L$(MSVC_LIB64) -L$(UniversalCRT_Lib64) -L$(WINSDK_LIB64)
	SYSTEM_FLAGS=-fmsc-version=$(_MSC_VER) -fms-extensions -fms-compatibility -fdelayed-template-parsing
	# The resulting commands
	C64=$(CLC) -c -isystem $(RTHDRS) $(TARGET_64) $(SYSTEM_INC_64)  $(SYSTEM_FLAGS)
	LINK64=lld-link $(SYSTEM_LIB_INC_64) /libpath:$(RTLIB)
else
	C64=$(CLC) -c -isystem $(RTHDRS) -march=x86-64 -fPIC -O3
	LINK64 := $(CLC) -lfoidlrt -L $(RTLIB)
endif

BHDRFLAGS := -I $(HDRS) $(RTHDRS) -a hdr
BCFLAGS := -I $(HDRS) $(RTHDRS)
LLCFLAGS = -march=x86-64 -x86-asm-syntax=intel -relocation-model=pic -O3 -filetype=obj

# Substitutions for recipe patterns

all: $(FOIDLC) | $(HDRS) $(LLS) $(OBJS)

ifeq ($(OS_NAME), Windows)
$(FOIDLC): $(FOBJS) $(FLLS) $(FHDRS) | $(BIN)
	$(LINK64) $(LIBS) /subsystem:console /out:$@ $(FOBJS)
else
$(FOIDLC): $(FHDRS) $(FOBJS) $(FLLS) | $(BIN)
	$(LINK64) $(FOBJS) -o $@
endif

$(BIN):
	mkdir bin

headers/%.defs : fsrc/%.foidl
	$(BSTRP) -s $< $(BHDRFLAGS) -o $@

$(FHDRS): | $(HDRS)

$(HDRS):
	mkdir headers

# Foidl LLVM-IR compilation

ll/%.ll : fsrc/%.foidl
	$(BSTRP) -s $< $(BCFLAGS) -o $@

$(FLLS): | $(LLS)

$(LLS):
	mkdir ll

# Foidl LLVM-IR compilation
objs/%.o : ll/%.ll
	$(C64) $< -o $@

$(FOBJS): | $(OBJS)

$(OBJS):
	mkdir objs


clean:
	$(RM) -r $(HDRS) $(LLS) $(OBJS) $(BIN)
