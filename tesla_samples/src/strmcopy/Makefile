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
LDPATH=$(PROJROOT)/lib/
LDFLAGS = -L $(LDPATH)
CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lbridge -lpthread

all: strmcopy.out strmcopy_dyn.out

strmcopy.out: strmcopy.c
	$(CC) $(CFLAGS) -o strmcopy.out strmcopy.c $(LIBS)

strmcopy_dyn.out: strmcopy_dyn.c
	$(CC) $(CFLAGS) -o strmcopy_dyn.out strmcopy_dyn.c $(LIBS)

install:
	$(INSTALL) -D strmcopy.out $(TARGETDIR)/dspbridge/strmcopy.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/strmcopy.out

	$(INSTALL) -D strmcopy_dyn.out $(TARGETDIR)/dspbridge/strmcopy_dyn.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/strmcopy_dyn.out

clean:
	\rm -f strmcopy.out strmcopy_dyn.out

