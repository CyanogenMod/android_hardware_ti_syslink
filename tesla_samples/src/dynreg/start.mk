#
#  dspbridge/make/start.mk
#
#  DSP-BIOS Bridge build rules.
#
#  Copyright (C) 2007 Texas Instruments, Inc.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation version 2.1 of the License.
#
#  This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
#  whether express or implied; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#


#  ----------------------------------------------------------------------------
#  Revision History
#
#  JUN 10, 2008    Removed		Ramesh Gupta G
#                  Kernel dependancy
#                  on sample building
#
#  MAY 16, 2002    REF=ORG    Sripal Bagadia
#  APR 02, 2004    REWRITE    Keith Deacon
#  ----------------------------------------------------------------------------

# make sure we have a prefix
ifndef PREFIX
$(error Error: variable PREFIX not defined)
endif

CMDDEFS =
CMDDEFS_START =


CROSS=arm-none-linux-gnueabi-
PROCFAMILY=OMAP_3430


ifndef PROCFAMILY
$(error Error: PROCFAMILY can not be determined from Kernel .config)
endif

ifndef TARGETDIR
TARGETDIR=$(PREFIX)/target
endif



#default (first) target should be "all"
#make sure the target directories are created
#all: $(HOSTDIR) $(ROOTFSDIR) $(TARGETDIR)
all: $(TARGETDIR)

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
RM    = rm $(1) 
MV    = mv $(1) $(2)
RMDIR = rm -r $(1)
MKDIR = mkdir -p $(1)
INSTALL = install

# Current Makefile directory
MAKEDIR := $(CURDIR)

# Implicit rule search not needed for *.d, *.c, *.h
%.d:
%.c:
%.h:

#   Tools
CC	:= $(CROSS)gcc
AR	:= $(CROSS)ar
LD	:= $(CROSS)ld
STRIP	:= $(CROSS)strip
