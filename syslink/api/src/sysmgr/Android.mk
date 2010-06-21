LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
SysMgr.c \
SysMgrDrv.c \
../procmgr/ProcMgr.c \
../procmgr/ProcMgrDrvUsr.c \
../procmgr/SysLinkMemUtils.c \
../procmgr/elfload/arm_dynamic.c \
../procmgr/elfload/arm_reloc.c \
../procmgr/elfload/ArrayList.c \
../procmgr/elfload/dload4430.c \
../procmgr/elfload/dload.c \
../procmgr/elfload/dload_endian.c \
../procmgr/elfload/dlw_client.c \
../procmgr/elfload/dlw_debug.c \
../procmgr/elfload/dlw_dsbt.c \
../procmgr/elfload/dlw_trgmem.c \
../procmgr/elfload/elf32.c \
../procmgr/elfload/symtab.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../include \
	$(LOCAL_PATH)/../../../samples/inc



LOCAL_SHARED_LIBRARIES += \
		libc \
		libipc \
		libipcutils \
		libnotify \
		libsysmemmgr

LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic
LOCAL_CFLAGS += -DSYSLINK_USE_SYSMGR
LOCAL_CFLAGS += -DARM_TARGET
LOCAL_MODULE    := libsysmgr

include $(BUILD_SHARED_LIBRARY)