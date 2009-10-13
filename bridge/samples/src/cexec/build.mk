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
#  ----------------------------------------------------------------------------
#
#  File    build.mk
#
#  Path    $(MAKEROOT)
#
#  Desc    This file defines information used to build this module.
#
#  Rev     0.1
#
#  ----------------------------------------------------------------------------

# ALL PATHS IN MAKEFILE MUST BE RELATIVE TO ITS DIRECTORY

CDEFS       += $(CMDDEFS) # Add command line definitions
CDEFS	    += $(PROC_DEF) # Processor Family e.g. 1510,1610

#   ----------------------------------------------------------
#   REMOVE LEADING AND TRAILING SPACES FROM MAKEFILE MACROS
#   ----------------------------------------------------------

TARGETNAME  := $(strip $(TARGETNAME))
TARGETTYPE  := $(strip $(TARGETTYPE))
SUBMODULES  := $(strip $(SUBMODULES))
SOURCES     := $(strip $(SOURCES))
INCLUDES    := $(strip $(INCLUDES))
LIBINCLUDES := $(strip $(LIBINCLUDES))

SH_SONAME   := $(strip $(SH_SONAME))
ST_LIBS     := $(strip $(ST_LIBS))
SH_LIBS     := $(strip $(SH_LIBS))

CFLAGS      := $(strip $(CFLAGS))
CDEFS       := $(strip $(CDEFS))
EXEC_ARGS   := $(strip $(EXEC_ARGS))
ST_LIB_ARGS := $(strip $(ST_LIB_ARGS))
SH_LIB_ARGS := $(strip $(SH_LIB_ARGS))

#   ----------------------------------------------------------
#   COMPILER OPTIONS
#   ----------------------------------------------------------

# Preprocessor : dependency file generation
ifndef NODEPENDS
ifndef nodepends
CFLAGS += -MD
endif
endif

#   Overall
CFLAGS += -pipe
#   Preprocessor
CFLAGS +=
#   Debugging
ifeq ($(BUILD),deb)
CFLAGS += -g
else
CFLAGS += -fomit-frame-pointer
endif
#   Warnings
CFLAGS += -Wall  -Wno-trigraphs -Werror-implicit-function-declaration #-Wno-format
#   Optimizations
CFLAGS += -O2 -fno-strict-aliasing
#   Machine dependent
ifeq ($(PROC_DEF),OMAP_1510)
CFLAGS += -mapcs-32 -march=armv4 -mtune=arm9tdmi -mshort-load-bytes -msoft-float
endif
ifeq ($(PROC_DEF), OMAP_16xx)
CFLAGS += -mapcs-32 -march=armv4 -mtune=arm9tdmi -mshort-load-bytes -msoft-float
endif
ifeq ($(PROC_DEF), OMAP_24xx)
CFLAGS += -mapcs-32 -malignment-traps -msoft-float
endif
#   Code generation
CFLAGS += -fno-common
#   Macros
CFLAGS += -DLINUX $(addprefix -D, $(CDEFS))

ifdef __KERNEL__
CFLAGS      += -D__KERNEL__  -fno-builtin
endif

#   ----------------------------------------------------------
#   OBJECTS
#   ----------------------------------------------------------

BUILDROOT   := $(LINUXROOT)/build/$(CONFIG)/$(BUILD)
BUILDDIR    := $(BUILDROOT)$(MAKEDIR)/
TARGETDIR   := $(LINUXROOT)/release/$(CONFIG)/$(BUILD)/

TARGET      := $(TARGETDIR)$(TARGETNAME)
LIBINCLUDES += $(TARGETDIR) $(TGTROOT)/lib $(TGTROOT)/usr/lib

SRCDIRS :=  $(sort $(dir $(SOURCES)))
OBJDIRS :=  $(addprefix $(BUILDDIR),$(SRCDIRS))

BASEOBJ := $(addsuffix .o,$(basename $(SOURCES)))
OBJECTS := $(addprefix $(BUILDDIR), $(BASEOBJ))

ST_LIBNAMES := $(addsuffix .a, $(addprefix lib, $(ST_LIBS)))
DL_LIBNAMES := $(addsuffix .so, $(addprefix lib, $(SH_LIBS)))

vpath %.a $(LIBINCLUDES) $(TGTROOT)/lib $(TGTROOT)/usr/lib
vpath %.so $(LIBINCLUDES) $(TGTROOT)/lib $(TGTROOT)/usr/lib

#   ----------------------------------------------------------
#   BUILD ARGUMENTS
#   ----------------------------------------------------------

MAPFILE := -Wl,-cref,-Map,$(TARGET).map
INCPATH := $(addprefix -I, $(INCLUDES))
LIBPATH := $(addprefix -L, $(LIBINCLUDES))
LIBFILE := $(addprefix -l, $(ST_LIBS) $(SH_LIBS))

ifeq ($(TARGETTYPE),SH_LIB)
CFLAGS += -fpic
TARGETARGS := $(SH_LIB_ARGS) -nostartfiles -nodefaultlibs -nostdlib -shared -Wl
ifneq ($(SH_SONAME),)
TARGETARGS += -Wl,-soname,$(SH_SONAME)
endif
endif

ifeq ($(TARGETTYPE),ST_LIB)
TARGETARGS := $(ST_LIB_ARGS) -nostartfiles -nodefaultlibs -nostdlib -Wl,-r
endif

ifeq ($(TARGETTYPE),EXEC)
TARGETARGS := $(EXEC_ARGS)
endif

.PHONY  :   all $(SUBMODULES) clean cleantrg SHELLERR Debug

#   ==========================================================
#   all
#   ==========================================================
all :  $(CHECKSHELL) $(SUBMODULES)

#   ==========================================================
#   Make submodules
#   ==========================================================
$(SUBMODULES):
ifndef NORECURSE
ifndef norecurse
	$(MAKE) -C $@ $(filter-out $(SUBMODULES),$(MAKECMDGOALS))
endif
endif

ifneq ($(TARGETTYPE),)

all :  $(OBJDIRS) $(TARGETDIR) $(TARGET)

#   ==========================================================
#   Create directories
#   ==========================================================
$(OBJDIRS) $(TARGETDIR) :
	@$(call MKDIR, $@)

#   ==========================================================
#   Build target
#   ==========================================================
$(TARGET):$(OBJECTS) $(ST_LIBNAMES) $(DL_LIBNAMES)
#   @$(SHELLCMD) echo Building $@
	cd $(BUILDDIR) && $(CC) $(TARGETARGS) $(CFLAGS) $(LIBPATH) $(MAPFILE) -o $@ $(BASEOBJ) $(LIBFILE)
ifdef RELEASEDIR
ifeq ($(BUILD),deb)
	$(ARCH_PREFIX)strip -go $(RELEASEDIR)/$(notdir $@) $@
else
	$(ARCH_PREFIX)strip --strip-unneeded -xgo $(RELEASEDIR)/$(notdir $@) $@
endif
#   cp $@ $(RELEASEDIR)
endif

#   ==========================================================
#   Compile .c file
#   ==========================================================
$(BUILDDIR)%.o:%.c
#   echo Compiling $(patsubst $(BUILDDIR)%.o,%.c, $@)
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $(patsubst $(BUILDDIR)%.o,%.c, $@)

#   ==========================================================
#   Compile .S file
#   ==========================================================
$(BUILDDIR)%.o:%.S
#   echo Compiling $(patsubst $(BUILDDIR)%.o,%.S, $@)
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $(patsubst $(BUILDDIR)%.o,%.S, $@)

endif   # ifneq ($(TARGETTYPE),)

#   ----------------------------------------------------------
#   clean - Remove build directory and target files
#   Linux : Removes object and dependency files in build folder
#   DOS   : Removes object dirs in build folder
#   ----------------------------------------------------------
clean : $(SUBMODULES)
ifneq ($(TARGETTYPE),)
ifneq ($(OBJECTS),)
	- @cd $(BUILDDIR) && \
	for cleanfile in $(BASEOBJ:.o=.*); do rm $$cleanfile; done
#   Used .* above instead of .o and .d to reduce items
#   - @for cleanfile in $(OBJDIRS); do rm $$cleanfile*; done
endif
	- @$(call RM, $(TARGET))
	- @$(call RM, $(TARGET).map)
endif

cleantrg : $(SUBMODULES)
ifneq ($(TARGETTYPE),)
	- @$(call RM, $(TARGET))
	- @$(call RM, $(TARGET).map)
endif

#   ----------------------------------------------------------
#   Include dependency files generated by preprocessor.
#
#   Dependency files are placed in main object directory because
#   dependent files' paths for same source file varies with the
#   directory from where gmake is run
#   ----------------------------------------------------------
ifndef NODEPENDS
ifndef nodepends
ifneq ($(OBJECTS),)
-include $(OBJECTS:.o=.d)
endif
endif
endif

#   ----------------------------------------------------------
#   Generate fatal error if make variable SHELL is incorrect
#   ----------------------------------------------------------
SHELLERR::
	@$(SHELLCMD) echo Fatal error: SHELL set to $(SHELL) instead of $(MYSHELL)
	@$(SHELLCMD) echo set $(MYSHELL) to correct path and CASE SENSITIVE FILE NAME and EXTENSTION
	@$(SHELLCMD) echo of your command shell
	$(ERR)


#   ----------------------------------------------------------
#   For debugging script
#   ----------------------------------------------------------
Debug::$(SUBMODULES)
	@$(SHELLCMD) echo SHELL: $(SHELL)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo CDEFS: $(CDEFS)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo CONFIG_SHELL: $(CONFIG_SHELL)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo CURDIR: $(CURDIR)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo SRCDIRS: $(SRCDIRS)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo OBJDIRS: $(OBJDIRS)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo OBJECTS: $(OBJECTS)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo BUILDDIR: $(BUILDDIR)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo TARGETDIR TARGETNAME: $(TARGET)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo MAKEDIR: $(MAKEDIR)
	@$(SHELLCMD) echo
	@$(SHELLCMD) echo INCLUDES: $(INCLUDES)
	@$(SHELLCMD) echo
