# ------------------------------------------------------------------------------
# Copyright 2019 Frank V. Castellucci
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
	export OS_NAME := $(UNAME)
else ifeq ($(UNAME), Solaris)
	export OS_NAME := $(UNAME)
else ifeq ($(UNAME), Darwin)
	export OS_NAME := $(UNAME)
else
	export OS_NAME := Windows
endif

$(info Building for $(OS_NAME))

ifeq ($(OS_NAME), Windows)
	export SHFOIDLC := ../foidlc/bin/foidlc.exe
	export PYTHON := python
else
	export SHFOIDLC := ../foidlc/bin/foidlc
	export PYTHON := python3
endif

export CLC    := clang
export CPPLC  := clang++
export CLINK  := lld-link
export RTHDRS := ../foidlrtl/headers/
export RTLIB  := ../foidlrtl/lib
export RTINC  := ../foidlrtl/includes
export EXTS_LIB := lib/
export EXTS_HDR := headers/


TOPTARGETS := all clean

SUBDIRS := http_curl

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)