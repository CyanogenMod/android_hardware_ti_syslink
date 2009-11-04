#
#  Syslink/src/api/procmgr/Makefile.am
#
#  Syslink procmgr api library Makefile.am file.
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

ACLOCAL_AMFLAGS=-I m4

PROJROOT = ../../..

AM_CFLAGS = -DSYSLINK_USE_SYSMGR

SUBDIRS  = procmgr4430

lib_LTLIBRARIES  = libprocmgr.la


libprocmgr_la_SOURCES     = \
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

libprocmgr_la_CPPFLAGS    = \
	-I$(PROJROOT)/api/include -DARM_TARGET $(AM_CLFAGS)

libprocmgr_la_LIBTOOLFLAGS = --tag=disable-static
libprocmgr_la_LDFLAGS = -version-info 0:0:0
libprocmgr_la_LIBADD = \
$(PROJROOT)/api/src/utils/libutils.la \
$(PROJROOT)/api/src/sysmgr/libsysmgr.la

