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
	OS_NAME := $(UNAME)
	PYTHON := python3
else ifeq ($(UNAME), Solaris)
	OS_NAME := $(UNAME)
else ifeq ($(UNAME), Darwin)
	OS_NAME := $(UNAME)
	PYTHON := python3
else
	OS_NAME := Windows
	PYTHON := python
endif

$(info Building for $(OS_NAME))

TOPTARGETS := all clean

SUBDIRS := http_curl

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(info $(PWD))
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)