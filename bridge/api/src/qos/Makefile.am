#
#  dspbridge/src/qos/Makefile.am
#
#  DSP-BIOS Bridge qos library Makefile.am file.
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

PROJROOT = ../..

#TODO:debug has to be endabled conditionally
if DEBUG
AM_CFLAGS = -DDEBUG
endif

lib_LIBRARIES = libqos.a
libqos_a_SOURCES = \
QOSComponent.c \
QOSResource.c \
QOSRegistry.c \
qosti.c


libqos_a_CFLAGS = \
	-I$(PROJROOT)/inc \
	-I$(PROJROOT)/inc/linux \
	-I$(PROJROOT)/src/api \
	-I./inc \
	$(AM_CFLAGS)


