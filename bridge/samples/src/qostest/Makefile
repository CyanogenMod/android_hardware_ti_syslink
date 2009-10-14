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
LDFLAGS = -L $(LDPATH)  -L $(PROJROOT)/src/qos
CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lbridge -lpthread -lqos

all: qostest.out

qostest.out: qostest.c
	$(CC) $(CFLAGS) -o qostest.out qostest.c $(LIBS)

install: qostest.out
	$(INSTALL) -D $< $(TARGETDIR)/dspbridge/$<
	$(STRIP) -s $(TARGETDIR)/dspbridge/$<

clean:
	\rm -f qostest.out
