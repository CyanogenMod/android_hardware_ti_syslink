#
#  Copyright 2001-2009 Texas Instruments - http://www.ti.com/
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

PROJROOT=$(top_srcdir)/samples
INCLUDE = \
	-I $(PROJROOT)/../api/include \
	-I $(PROJROOT)/inc


AM_CFLAGS = -Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY)
AM_CFLAGS += -DSYSLINK_TRACE_ENABLE
#AM_CFLAGS += -DSYSLINK_USE_SYSMGR
LDPATH=../../../../api/src

API_LIBS_NONSYS = \
	$(LDPATH)/utils/libutils.la \
	$(LDPATH)/ipc/libipc.la \
	$(LDPATH)/notify/libnotify.la\
	$(LDPATH)/procmgr/procmgr4430/libomap4430proc.la \
	$(LDPATH)/procmgr/libprocmgr_nonsysmgr.la

API_LIBS = \
	$(LDPATH)/utils/libutils.la \
	$(LDPATH)/ipc/libipc.la \
	$(LDPATH)/notify/libnotify.la \
	$(LDPATH)/procmgr/libprocmgr.la \
	$(LDPATH)/sysmgr/libsysmgr.la \
	$(LDPATH)/sysmemmgr/libsysmemmgr.la

bin_PROGRAMS = heapBufApp.out
#heapBufApp_NonSysMgr.out

heapBufApp_out_SOURCES = HeapBufAppOS.c HeapBufApp.c

heapBufApp_out_CPPFLAGS = $(AM_CLFAGS) -DSYSLINK_USE_SYSMGR

heapBufApp_out_LDADD = $(API_LIBS)

#heapBufApp_NonSysMgr_out_SOURCES = HeapBufAppOS.c HeapBufApp.c

#heapBufApp_NonSysMgr_out_CPPFLAGS = $(AM_CLFAGS)

#heapBufApp_NonSysMgr_out_LDADD = $(API_LIBS_NONSYS)
