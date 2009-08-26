#
#  Copyright 2001-2008 Texas Instruments - http://www.ti.com/
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

PROJROOT=../..

include $(PROJROOT)/make/start.mk

INCLUDE=-I $(PROJROOT)/../api/include -I $(PROJROOT)/inc
LDPATH=$(PROJROOT)/../target/lib $(PROJROOT)/../target/usr/lib
LDFLAGS = $(addprefix -L, $(LDPATH))

CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lutils -lipc -lprocmgr -lomap4430proc

all: ducati_load.out

ducati_load.out: ducati_load.c getopt.c
	$(CC) $(CFLAGS) -o ducati_load.out ducati_load.c getopt.c $(LIBS) 

install: ducati_load.out
	$(INSTALL) -D $< $(TARGETDIR)/procmgrapps//$<
	$(STRIP) -s $(TARGETDIR)/procmgrapps/$<

clean:
	\rm -f ducati_load.out
