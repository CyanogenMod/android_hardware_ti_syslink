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
#   @brief  Makefile for user-side RCM Multi Test samples
#
#   ============================================================================

PROJROOT = ../..

include $(PROJROOT)/make/start.mk

INCLUDE=-I $(PROJROOT)/../api/include -I $(PROJROOT)/inc
LDPATH=$(PROJROOT)/../target/lib $(PROJROOT)/../target/usr/lib
LDFLAGS = $(addprefix -L, $(LDPATH))

CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)
CFLAGS += -DSYSLINK_USE_SYSMGR
#CFLAGS += -DSYSLINK_USE_LOADER


LIBS = -lutils -lnotify -lipc -lprocmgr -lrcm -lpthread -lrt
LIBS += -lsysmgr -lsysmemmgr

all: rcm_multitest.out rcm_multithreadtest.out rcm_multiclienttest.out

rcm_multitest.out:
	$(CC) $(CFLAGS) -o rcm_multitest.out RcmClientServerTest.c $(LIBS)

rcm_multithreadtest.out:
	$(CC) $(CFLAGS) -o rcm_multithreadtest.out RcmMultiThreadTest.c $(LIBS)

rcm_multiclienttest.out:
	$(CC) $(CFLAGS) -o rcm_multiclienttest.out RcmMultiClientTest.c $(LIBS)

install1: rcm_multitest.out
	$(INSTALL) -D $< $(TARGETDIR)/syslink/$<
	$(STRIP) -s $(TARGETDIR)/syslink/$<

install2: rcm_multithreadtest.out
	$(INSTALL) -D $< $(TARGETDIR)/syslink/$<
	$(STRIP) -s $(TARGETDIR)/syslink/$<

install3: rcm_multiclienttest.out
	$(INSTALL) -D $< $(TARGETDIR)/syslink/$<
	$(STRIP) -s $(TARGETDIR)/syslink/$<

install: install1 install2 install3

clean:
	\rm -f rcm_multitest.out
	\rm -f rcm_multithreadtest.out
	\rm -f rcm_multiclienttest.out
