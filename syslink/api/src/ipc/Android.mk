LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm



LOCAL_SRC_FILES := \
IPCManager.c \
MultiProcDrv.c \
MultiProc.c \
SharedRegionDrv.c \
SharedRegion.c\
GatePetersonDrv.c \
GatePeterson.c \
HeapBufDrv.c \
HeapBuf.c \
NameServerDrv.c \
NameServer.c \
NameServerRemoteNotifyDrv.c \
NameServerRemoteNotify.c \
ListMP.c \
ListMPSharedMemoryDrv.c \
ListMPSharedMemory.c \
MessageQDrv.c \
MessageQ.c \
MessageQTransportShmDrv.c \
MessageQTransportShm.c \
GateHWSpinLockDrv.c \
GateHWSpinLock.c \


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../include \
	$(LOCAL_PATH)/../../../samples/inc


LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic

LOCAL_SHARED_LIBRARIES += \
		libc \
		libipcutils

LOCAL_MODULE    := libipc

include $(BUILD_SHARED_LIBRARY)