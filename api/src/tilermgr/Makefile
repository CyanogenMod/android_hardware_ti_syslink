PROJROOT = ../../..

include $(PROJROOT)/api/make/start.mk

# Do not change above "include" line(s)

# Arguments to tools, will move to make system once finalized.

CFLAGS         = 
CDEFS          = 
ifeq ($(BUILD),udeb)
CDEFS          += DEBUG 
endif

EXEC_ARGS      = 
ST_LIB_ARGS    = 
SH_LIB_ARGS    = 

# Define this macro if target runs in kernel mode
#__KERNEL__ = 1

# Target name and extension
# static library        (ST_LIB): filename.a
# shared library soname (SH_LIB): filename.so.maj_ver.min_ver
# executable            (EXEC)  : filename.out

TARGETNAME  = libdmmdrv


# TARGETTYPE must be EXEC, ST_LIB or SH_LIB in upper case.

TARGETTYPE  = SH_LIB

# install directory relative to the HOSTTARGET directory
HOSTRELEASE = lib

# install directory relative to the root filesystem
ROOTFSRELEASE = lib

# For shared object library, soname is filename.so.maj_ver

SH_SONAME = libdmmdrv.so.0


# Folders in which gmake will run before building current target

SUBMODULES  = \

# Filename must not begin with '.', '/' or '\'

SOURCES     = \
	tilermgr.c

# Search path for include files

INCLUDES    = \
    $(PROJROOT)/api/include \

# Libraries needed for linking.

ST_LIBS        =
SH_LIBS        = pthread utils ipc rcm

# Search path for library (and linker command) files.
# Current folder and target folder are included by default.

LIBINCLUDES += $(PROJROOT)/api/src/utils \
               $(PROJROOT)/api/src/ipc \
               $(PROJROOT)/api/src/rcm

# Do not change below "include" line(s)

include $(PROJROOT)/api/make/build.mk

