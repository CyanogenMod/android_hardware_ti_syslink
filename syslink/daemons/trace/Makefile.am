#
#  syslink/daemons/trace/Makefile.am
#
#  SysLink Trace Buffer daemon's Makefile.am file.
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

PROJROOT = $(top_srcdir)/daemons
AM_CFLAGS = -DSYSLINK_USE_LOADER
#AM_CFLAGS += -DSYSLINK_TRACE_ENABLE

bin_PROGRAMS = syslink_trace_daemon.out

syslink_trace_daemon_out_SOURCES = \
SyslinkTraceDaemon.c

syslink_trace_daemon_out_CPPFLAGS    = \
	-I$(PROJROOT)/../api/include \
	 $(AM_CFLAGS)

API_SRCDIR = ../../api/src

syslink_trace_daemon_out_LDADD = \
	$(API_SRCDIR)/utils/libipcutils.la
