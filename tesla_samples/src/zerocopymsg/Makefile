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
LDPATH=$(PROJROOT)/lib
LDFLAGS = -L $(LDPATH)
CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lbridge -lpthread

all: zerocopymsg.out

zerocopymsg.out: zerocopymsg.c
	$(CC) $(CFLAGS) -o zerocopymsg.out zerocopymsg.c $(LIBS)


install:
	$(INSTALL) -D zerocopymsg.out $(TARGETDIR)/dspbridge/zerocopymsg.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/zerocopymsg.out


clean:
	\rm -f zerocopymsg.out

