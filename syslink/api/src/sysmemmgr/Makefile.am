#
#  syslink/api/src/sysmemgr/Makefile.a,
#
#  SysLink SysMemMgr API library Makefile.am file.
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

lib_LTLIBRARIES = libsysmemmgr.la
libsysmemmgr_la_SOURCES = \
SysMemMgr.c \
SysMemMgrDrv.c

libsysmemmgr_la_CFLAGS = \
	-I$(PROJROOT)/api/include

libsysmemmgr_la_LIBTOOLFLAGS = --tag=disable-static
libsysmemmgr_la_LDFLAGS = -version-info 0:0:0
