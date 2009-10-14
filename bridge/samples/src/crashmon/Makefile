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
#

PROJROOT=../..

include $(PROJROOT)/make/start.mk

INCLUDE=-I $(PROJROOT)/inc -I $(PROJROOT)/../TARGET/host/include
LDPATH=$(TARGETDIR)/lib
LDFLAGS = -L $(LDPATH)
CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lbridge -lpthread

all: crashmon.out faultapp.out

crashmon.out: CrashMon.c
	$(CC) $(CFLAGS) -o crashmon.out CrashMon.c $(LIBS)

faultapp.out: faultapp.c
	$(CC) $(CFLAGS) -o faultapp.out faultapp.c $(LIBS)

install:
	
	$(INSTALL) -D crashmon.out $(TARGETDIR)/dspbridge/crashmon.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/crashmon.out
	
	$(INSTALL) -D faultapp.out $(TARGETDIR)/dspbridge/faultapp.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/faultapp.out

clean:
	\rm -f crashmon.out faultapp.out


