#
#  Copyright 2001-2008 Texas Instruments - http://www.ti.com/
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  ----------------------------------------------------------------------------
#
#  File    start.mk
#
#  Path    $(MAKEROOT)
#
#  Desc    This file defines information used to build this module.
#
#  Rev     0.1
#
#  ----------------------------------------------------------------------------

#default (first) target should be "all"
all:


CONFIG_SHELL := /bin/bash

SHELL := $(CONFIG_SHELL)

# Current version of gmake (3.79.1) cannot run windows shell's internal commands
# We need to invoke command interpreter explicitly to do so.
# for winnt it is cmd /c <command>
SHELLCMD:=

ifneq ($(SHELL),$(CONFIG_SHELL))
CHECKSHELL:=SHELLERR
else
CHECKSHELL:=
endif

# Error string to generate fatal error and abort gmake
ERR = $(error Makefile generated fatal error while building target "$@")

CP  :=   cp

MAKEFLAGS = r

QUIET := &> /dev/null

# Should never be :=
RM    = rm $(1) $(QUIET)
MV    = mv $(1) $(2)
RMDIR = rm -r $(1)
MKDIR = mkdir -p $(1)

#CONFIG ?= arm4
#BUILD  ?= deb

# Current Makefile directory
MAKEDIR := $(CURDIR)

# Implicit rule search not needed for *.d, *.c, *.h
%.d:
%.c:
%.h:

#   Tools
ifeq ($(MVLPROD),mvlpe)
ARCH_PREFIX := arm_920t_le-
else
ARCH_PREFIX := arm-linux-
endif
CC  :=   $(ARCH_PREFIX)gcc
AR  :=   $(ARCH_PREFIX)ar
LD  :=   $(ARCH_PREFIX)ld

