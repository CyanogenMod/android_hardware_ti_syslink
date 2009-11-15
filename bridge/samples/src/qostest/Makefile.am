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
INCLUDE= \
	-I $(PROJROOT)/inc \
	-I $(PROJROOT)/../target/host/include


AM_CFLAGS = -Wall -g -O2 $(INCLUDE) -finline-functions -D$(PROCFAMILY)

API_LIBS = \
	../../../api/src/bridge/libbridge.la \
	../../../api/src/qos/libqos.a


bin_PROGRAMS = qostest.out

qostest_out_SOURCES = qostest.c
qostest_out_CPPFLAGS = $(AM_CLFAGS)
qostest_out_LDADD = $(API_LIBS)

