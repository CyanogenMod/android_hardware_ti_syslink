#
#  dspbridge/src/api/Makefile.am
#
#  DSP-BIOS Bridge api library Makefile.am file.
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
#
#      REF=ORG
#      Original version.
#  ----------------------------------------------------------------------------

PROJROOT = $(top_srcdir)/api

if DEBUG
AM_CFLAGS = -DDEBUG
endif

lib_LTLIBRARIES = libbridge.la
libbridge_la_SOURCES = \
DSPManager.c \
DSPProcessor.c \
DSPProcessor_OEM.c \
DSPNode.c \
DSPStrm.c \
perfutils.c \
dsptrap.c


libbridge_la_CFLAGS = \
	-I$(PROJROOT)/inc \
	-I$(PROJROOT)/inc/linux \
	$(AM_CFLAGS)
libbridge_la_LIBTOOLFLAGS = --tag=disable-static
libbridge_la_LDFLAGS = -version-info 0:0:0
