# only include if running on an omap4 platform
ifeq ($(TARGET_BOARD_PLATFORM),omap4)
include $(call first-makefiles-under,$(call my-dir))
endif
