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

$(info Building for $(OS_NAME))
ifeq ($(OS_NAME), Windows)
SHFOIDLC := ../../foidlc/bin/foidlc.exe
else
SHFOIDLC := ../../foidlc/bin/foidlc
endif

# llvm toolchain executables

LLLT 	= llvm-lib
LLAR	= llvm-ar
CLC 	= clang
CPPLC   = clang++

# FOIDL Runtime generation settings

FSRC  	:= fsrc/
SRC  	:= src/
LLS 	:= ll/
OBJS 	:= objs/
LIB 	:= ../lib/
HDRS   	:= ../headers/
RTHDRS  := ../../foidlrtl/headers/
RTINC 	:= ../../foidlrtl/includes/

# Substitutions for recipe patterns
FOIDL  	:= .foidl
LL 		:= .ll
OBJ 	:= .o

ifeq ($(DEBUG), 1)
$(info Building for debugging)
DFLAGS := -O0 -g
else
$(info Building for release)
DFLAGS :=
endif

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
FOBJS 	:= $(subst $(FSRC), $(OBJS), $(FSRCS:$(FOIDL)=$(OBJ)))

ifeq ($(OS_NAME), Windows)
	ARCH32=x86
	ARCH64=x64
	ARCH=$(ARCH64) # set to either one, and the right stuff will get chosen
	DEFINES=-DWIN32 -std=c++11 #-fno-rtti -fno-exceptions -D_HAS_EXCEPTIONS=0 -D_ITERATOR_DEBUG_LEVEL=0
	_MSC_VER=1900 # 1800=VC2013, 1900=VC2015, 1910=VC2017 / others: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
	# LIBS=libcmt.lib

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
	CL64=$(CLC) -c $(TARGET_64) $(DFLAGS)
	C64=$(CLC) -c -isystem $(RTINC) $(TARGET_64)  $(SYSTEM_INC_64) -D_CRT_SECURE_NO_WARNINGS -std=c11 $(SYSTEM_FLAGS) $(DFLAGS)
	CPP64=$(CPPLC) -c -isystem $(RTINC) $(TARGET_64) $(SYSTEM_INC_64) -D_CRT_SECURE_NO_WARNINGS -std=c++14 $(SYSTEM_FLAGS) $(DFLAGS)
	ext_LIB  := libhttp_curl.lib
	STATIC_LIB  := $(LIB)$(ext_LIB)
	LIB64=$(LLLT) $(SYSTEM_LIB_INC_64) /out:$(STATIC_LIB) /nologo
else
	CL64=$(CLC) -c -march=x86-64 -pthread -fPIC $(DFLAGS)
	C64=$(CLC) -c -isystem $(RTINC) -pthread -march=x86-64 -fPIC $(DFLAGS)
	CPP64=$(CPPLC) -c -isystem $(RTINC) -pthread -march=x86-64 -std=c++11 -fPIC $(DFLAGS)
	ext_LIB  := libhttp_curl.a
	STATIC_LIB  := $(LIB)$(ext_LIB)
	LIB64=$(LLAR) --format=default -ru $(STATIC_LIB)
endif

FDEFS 	:= .defs
FOIDL  	:= .foidl
LL 		:= .ll
OBJ 	:= .o
CSRC	:= .c
CPPSRC  := .cpp

# Flags for compilation of all types

BHDRFLAGS := -I $(HDRS) -r -a hdr
BCFLAGS := -I $(HDRS) -r

BHDRLIBFLAGS := -I $(HDRS) -a hdr
BLIBCFLAGS := -I $(HDRS)

# Substitutions for recipe patterns

FSRCS 	:= $(wildcard $(FSRC)*$(FOIDL))
FHDRS   := $(subst $(FSRC), $(HDRS), $(FSRCS:$(FOIDL)=$(FDEFS)))
FLLS	:= $(subst $(FSRC), $(LLS), $(FSRCS:$(FOIDL)=$(LL)))
FOBJS	:= $(subst $(FSRC), $(OBJS), $(FSRCS:$(FOIDL)=$(OBJ)))

SRCS	:= $(wildcard $(SRC)*$(CSRC))
SOBJS	:= $(subst $(SRC),$(OBJS), $(SRCS:$(CSRC)=$(OBJ)))

SRCPPS  := $(wildcard $(SRC)*$(CPPSRC))
SPPOBJS := $(subst $(SRC),$(OBJS),$(SRCPPS:$(CPPSRC)=$(OBJ)))

SHFLAGS := -I $(RTHDRS)

all: $(STATIC_LIB) | $(FLLS) $(FLIBLLS) $(OBJS)

# all: $(SOBJS)

$(STATIC_LIB): $(SOBJS) $(FOBJS) | $(LIB)
	$(LIB64) $(SOBJS) $(FOBJS)

$(LIB):
	mkdir lib

# Extension foidl Source -> headers and -> ll

headers/%.defs : fsrc/%.foidl
	$(SHFOIDLC) -c $< $(BHDRFLAGS) -o $@

$(FHDRS):

ll/%.ll : fsrc/%.foidl $(SHFOIDLC)
	$(SHFOIDLC) $(SHFLAGS) -c $<  -o $@

$(FLLS): | $(FHDRS) $(LLS)

$(LLS):
	mkdir ll

# Foidl LLVM-IR compilation

objs/%.o : ll/%.ll
	$(CL64) $< -o $@

$(FOBJS): | $(OBJS)

# CPP file compilations

$(OBJS)%.o: $(SRC)%.cpp
	$(CPP64) $< -o $@

$(SPPOBJS): | $(OBJS)


# C file compilations

$(OBJS)%.o: $(SRC)%.c
	$(C64) $< -o $@

$(OBJS)%.o: $(NSRC)%.c
	$(C64) $< -o $@

$(SOBJS): | $(OBJS)

$(OBJS):
	mkdir objs

clean:
	$(RM) -r $(FHDRS) $(FLIBHDRS) $(LLS) $(OBJS) $(LIB)