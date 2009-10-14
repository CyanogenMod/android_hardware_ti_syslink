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
LDPATH=$(TARGETDIR)/lib $(TARGETDIR)/usr/lib
LDFLAGS = $(addprefix -L, $(LDPATH))
CFLAGS=-Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY) $(LDFLAGS)

LIBS = -lbridge -lpthread

all: scale.out scale_dyn.out

scale.out: scale.c
	$(CC) $(CFLAGS) -o scale.out scale.c $(LIBS)

scale_dyn.out: scale_dyn.c
	$(CC) $(CFLAGS) -o scale_dyn.out scale_dyn.c $(LIBS)

install:
	
	$(INSTALL) -D scale.out $(TARGETDIR)/dspbridge/scale.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/scale.out
	
	$(INSTALL) -D scale_dyn.out $(TARGETDIR)/dspbridge/scale_dyn.out
	$(STRIP) -s $(TARGETDIR)/dspbridge/scale_dyn.out

clean:
	\rm -f scale.out scale_dyn.out

