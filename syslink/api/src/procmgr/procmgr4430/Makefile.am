#
#  Syslink/src/api/procmgr/procmgr4430/Makefile.am
#
#  Syslink procmgr4430 api library Makefile.am file.
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

PROJROOT = ../../../..

lib_LTLIBRARIES = libomap4430proc.la
libomap4430proc_la_SOURCES = \
omap4430proc.c \
omap4430procDrvUsr.c

libomap4430proc_la_CFLAGS = \
	-I$(PROJROOT)/api/include \
	-I$(PROJROOT)/api/include/linux \
	-I$(PROJROOT)/api/src/procmgr

libomap4430proc_la_LIBTOOLFLAGS = --tag=disable-static
libomap4430proc_la_LDFLAGS = -version-info 0:0:0
libomap4430proc_la_LIBDADD = $(PROJROOT)/api/src/utils/libutils.la
