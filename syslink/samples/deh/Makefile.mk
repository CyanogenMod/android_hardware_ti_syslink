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

#   ============================================================================
#   @file   Makefile
#
#   @brief  Makefile for SysLink DEH sample
#
#   ============================================================================

PROJROOT=../

include $(PROJROOT)/make/start.mk

INCLUDE=-I $(PROJROOT)/../api/include -I $(PROJROOT)/inc
LDPATH=$(TARGETDIR)/lib $(TARGETDIR)/usr/lib
LDFLAGS = $(addprefix -L, $(LDPATH))

CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)
CFLAGS += -DSYSLINK_TRACE_ENABLE

LIBS = -lipcutils -lsyslinknotify -lipc -lprocmgr
LIBS += -lsysmgr
MEMMGRLIBS = -ltimemmgr

all: dehtest dehdaemontest

dehtest: dehOS.c dehApp.c
	$(CC) $(CFLAGS) -DSYSLINK_USE_LOADER -o dehtest dehOS.c dehApp.c $(LIBS) $(MEMMGRLIBS)

dehdaemontest: dehOS.c dehApp.c
	$(CC) $(CFLAGS) -DSYSLINK_USE_DAEMON -o dehdaemontest dehOS.c dehApp.c $(LIBS) $(MEMMGRLIBS)

dehtestinstall1: dehtest
	$(INSTALL) -D $< $(TARGETDIR)/syslink/$<
	$(STRIP) -s $(TARGETDIR)/syslink/$<

dehtestinstall2: dehdaemontest
	$(INSTALL) -D $< $(TARGETDIR)/syslink/$<
	$(STRIP) -s $(TARGETDIR)/syslink/$<

install: dehtestinstall1 dehtestinstall2

clean:
	\rm -f dehtest
	\rm -f dehdaemontest
