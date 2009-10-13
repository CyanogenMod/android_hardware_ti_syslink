#
#  syslink/api/src/Makefile.am
#
#  SysLink API library Makefile.am.
#
#  Copyright (C) 2009 Texas Instruments, Inc.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation version 2.1 of the License.
#
#  This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
#  whether express or implied; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  ----------------------------------------------------------------------------
#  Revision History
#
#
#      REF=ORG
#      Original version.
#  ----------------------------------------------------------------------------

# Folders in which automake will run before building current target
if PROCFAMILY_IS_4430
SUBDIRS	= utils
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= notify
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= ipc
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= sysmgr
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= procmgr
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= sysmemmgr
endif

if PROCFAMILY_IS_4430
SUBDIRS	+= rcm
endif
