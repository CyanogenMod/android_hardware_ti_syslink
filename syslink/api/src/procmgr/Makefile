#
#
#  Copyright (C) 2009 Texas Instruments, Inc.
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
#
#      REF=ORG
#      Original version.
#  ----------------------------------------------------------------------------

PROJROOT = ../../..

include $(PROJROOT)/api/make/start.mk

# Do not change above "include" line(s)

# Arguments to tools, will move to make system once finalized.

CFLAGS         = -DSYSLINK_USE_SYSMGR
CDEFS          = ARM_TARGET
ifeq ($(BUILD),udeb)
CDEFS          += DEBUG 
endif

EXEC_ARGS      = 
ST_LIB_ARGS    = 
SH_LIB_ARGS    = 

# Define this macro if target runs in kernel mode
#__KERNEL__ = 1

# Target name and extension
# static library        (ST_LIB): filename.a
# shared library soname (SH_LIB): filename.so.maj_ver.min_ver
# executable            (EXEC)  : filename.out

TARGETNAME  = libprocmgr


# TARGETTYPE must be EXEC, ST_LIB or SH_LIB in upper case.

TARGETTYPE  = SH_LIB

# install directory relative to the HOSTTARGET directory
HOSTRELEASE = lib

# install directory relative to the root filesystem
ROOTFSRELEASE = lib

# For shared object library, soname is filename.so.maj_ver

SH_SONAME = libprocmgr.so.2

# Folders in which gmake will run before building current target

SUBMODULES  = \
procmgr4430

# Filename must not begin with '.', '/' or '\'

SOURCES     = \
ProcMgr.c \
ProcMgrDrvUsr.c \
SysLinkMemUtils.c \
elfload/arm_dynamic.c \
elfload/arm_reloc.c \
elfload/ArrayList.c \
elfload/dload.c \
elfload/dload_endian.c \
elfload/dolt_client.c \
elfload/dolt_debug.c \
elfload/dolt_dsbt.c \
elfload/dolt_trgmem.c \
elfload/elf32.c \
elfload/symtab.c



# Search path for include files

INCLUDES    = \
    $(PROJROOT)/api/include \
    $(PROJROOT)/api/inc \
    $(PROJROOT)/api/include/linux\

# Libraries needed for linking.

#SH_LIBS        = pthread utils
SH_LIBS        = utils sysmgr
ST_LIBS        = gcc

# Search path for library (and linker command) files.
# Current folder and target folder are included by default.

LIBINCLUDES += \
	  $(PROJROOT)/api/src/utils \
	  $(PROJROOT)/api/src/sysmgr
	


# Do not change below "include" line(s)

include $(PROJROOT)/api/make/build.mk
