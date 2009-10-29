#
#  syslink/daemons/syslink/Makefile.am
#
#  SysLink daemons syslink test Makefile.am file.
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

PROJROOT = ..
AM_CFLAGS = -DSYSLINK_USE_SYSMGR
#AM_CFLAGS += -DSYSLINK_TRACE_ENABLE
AM_CFLAGS += -DSYSLINK_USE_LOADER

bin_PROGRAMS = syslink_trace_daemon.out

syslink_trace_daemon_out_SOURCES = \
SyslinkTraceDaemon.c

syslink_trace_daemon_out_CPPFLAGS    = \
	-I$(PROJROOT)/inc \
	-I$(PROJROOT)/../api/include \
	-I$(TILER_INC_PATH) \
	 $(AM_CFLAGS)

API_SRCDIR = $(PROJROOT)/../api/src

syslink_trace_daemon_out_LDADD = \
	$(API_SRCDIR)/utils/libutils.la \
	$(API_SRCDIR)/procmgr/libprocmgr.la \
	$(API_SRCDIR)/ipc/libipc.la \
	$(API_SRCDIR)/rcm/librcm.la \
	$(API_SRCDIR)/notify/libnotify.la \
	$(API_SRCDIR)/sysmgr/libsysmgr.la \
	$(API_SRCDIR)/sysmemmgr/libsysmemmgr.la

