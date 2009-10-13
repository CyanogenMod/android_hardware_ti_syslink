
#  ----------------------------------------------------------------------------
#  Revision History
#
#
#  ----------------------------------------------------------------------------

PROJROOT=../..

include ./start.mk

# Do not change above "include" line(s)

# Arguments to tools, will move to make system once finalized.

CMDDEFS        =
CFLAGS         = 
#CDEFS          = __KERNEL__ LEAD3 
ifeq ($(PROCFAMILY),OMAP_2430)
CDEFS          = TMS32060 _DB_TIOMAP
else
ifeq ($(PROCFAMILY),OMAP_3430)
CDEFS          = TMS32060 _DB_TIOMAP
else
CDEFS          = LEAD3 _DB_TIOMAP
endif
endif
ifeq ($(BUILD),deb) # Additional flags for debug builds
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

TARGETNAME  = dynreg


# TARGETTYPE must be EXEC, ST_LIB or SH_LIB in upper case.

#TARGETTYPE  = ST_LIB
TARGETTYPE  = EXEC

# For shared object library, soname is filename.so.maj_ver

SH_SONAME = 

# Folders in which gmake will run before building current target

SUBMODULES  = \

# Filename must not begin with '.', '/' or '\'

SOURCES     = \

# Search path for include files

INCLUDES    = \
    $(KRNLSRC)/include \
    ../../inc \
    ../../inc/linux
#    ../../mpu/include

# Libraries needed for linking.

#ST_LIBS        = pthread bridge
#ST_LIBS        = bridge
#SH_LIBS        =
#SH_LIBS        = pthread bridge
LIB_OBJS        = -lpthread -lbridge


# Search path for library (and linker command) files.
SOURCES     = \
dynreg.c \
DLsymtab.c \
DLsymtab_support.c \
cload.c \
getsection.c \
reloc.c \
csl.c \
uuidutil.c \

# Current folder and target folder are included by default.
#	$(CURDIR)/../../mpu/lib/cee

LIBINCLUDES = $(TARGETDIR)
#LIBINCLUDES =  \
#	$(HOME)/omap/release/arm4/deb


# Do not change below "include" line(s)

include ./build.mk
