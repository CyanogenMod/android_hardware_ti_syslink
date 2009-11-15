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

if PROCFAMILY_IS_3430
CDEFS          = -DTMS32060 -D_DB_TIOMAP
else
CDEFS          = -DLEAD3 -D_DB_TIOMAP
endif
if DEBUG
CDEFS          += -DDEBUG
endif

#$(KRNLSRC)/include \ #TODO: check if this is required
INCLUDES = \
	-I$(PROJROOT)/inc \
	-I$(PROJROOT)/inc/linux

AM_CFLAGS = -Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(CDEFS)

API_LIBS = \
	../../../api/src/bridge/libbridge.la

bin_PROGRAMS = dynreg.out

dynreg_out_SOURCES = \
dynreg.c \
DLsymtab.c \
DLsymtab_support.c \
cload.c \
getsection.c \
reloc.c \
csl.c \
uuidutil.c

dynreg_out_CPPFLAGS = $(AM_CLFAGS)
dynreg_out_LDADD = $(API_LIBS)

EXTRA_DIST = build.mk start.mk
