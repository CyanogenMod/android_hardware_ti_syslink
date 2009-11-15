#
#  syslink/usr/src/ipc/Makefile.am
#
#  SysLink ipc API library Makefile.am.
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

PROJROOT = $(top_srcdir)

lib_LTLIBRARIES =libipc.la
libipc_la_SOURCES = \
IPCManager.c \
SharedRegionDrv.c \
SharedRegion.c\
GatePetersonDrv.c \
GatePeterson.c \
HeapBufDrv.c \
HeapBuf.c \
NameServerDrv.c \
NameServer.c \
NameServerRemoteNotifyDrv.c \
NameServerRemoteNotify.c \
ListMP.c \
ListMPSharedMemoryDrv.c \
ListMPSharedMemory.c \
MessageQDrv.c \
MessageQ.c \
MessageQTransportShmDrv.c \
MessageQTransportShm.c \
GateHWSpinLockDrv.c \
GateHWSpinLock.c \
MultiProcDrv.c \
MultiProc.c

libipc_la_LIBTOOLFLAGS = --tag=disable-static

libipc_la_CFLAGS = \
	-I$(PROJROOT)/api/include

