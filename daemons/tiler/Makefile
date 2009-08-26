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
#----------------------------------------------------------------------------

PROJROOT = ..

include $(PROJROOT)/make/start.mk

# Do not change above "include" line(s)

# Arguments to tools, will move to make system once finalized.

CFLAGS         = 
CDEFS          = 
ifeq ($(BUILD),udeb)
CDEFS          += DEBUG 
endif
#Uncomment below line to use sys manager
CFLAGS += -DSYSLINK_USE_SYSMGR
#CFLAGS += -DTILER_SYSM3

EXEC_ARGS      = 
ST_LIB_ARGS    = 
SH_LIB_ARGS    = 

# Define this macro if target runs in kernel mode
#__KERNEL__ = 1

# Target name and extension
# static library        (ST_LIB): filename.a
# shared library soname (SH_LIB): filename.so.maj_ver.min_ver
# executable            (EXEC)  : filename.out

TARGETNAME  = tiler_daemon


# TARGETTYPE must be EXEC, ST_LIB or SH_LIB in upper case.

TARGETTYPE  = EXEC

# install directory relative to the HOSTTARGET directory
HOSTRELEASE = target

# install directory relative to the root filesystem
ROOTFSRELEASE = target

# Folders in which gmake will run before building current target

SUBMODULES  = \

# Filename must not begin with '.', '/' or '\'

SOURCES     = \
TilerDaemon.c \
TilerMgrServer.c \

# Search path for include files

INCLUDES    = \
    $(PROJROOT)/inc \
    $(PROJROOT)/../api/include \

# Libraries needed for linking.

ST_LIBS        =
SH_LIBS        = pthread rt utils procmgr ipc rcm notify dmmdrv
#Uncomment the below line to use sys manager
SH_LIBS        += sysmgr sysmemmgr

# Search path for library (and linker command) files.
# Current folder and target folder are included by default.

LIBINCLUDES += $(PROJROOT)/../api/src/tiler/ 


# Do not change below "include" line(s)

include $(PROJROOT)/make/build.mk
 
