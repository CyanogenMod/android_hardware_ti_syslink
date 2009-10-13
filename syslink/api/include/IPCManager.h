/*
 * Syslink-IPC for TI OMAP Processors
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
/** ============================================================================
 *  @file   IPCManager.h
 *
 *  @brief      Defines the IPC driver wrapper for Syslink modules.
 *
 *  ============================================================================
 */

#ifndef IPCMANAGER_
#define IPCMANAGER_

#include <Std.h>

#undef open
#define open  IPCManager_open

#undef close
#define close IPCManager_close

#undef ioctl
#define ioctl IPCManager_ioctl

#undef fcntl
#define fcntl IPCManager_fcntl


/*
 *  ======== IPCManager_open ========
 *      Open handle to the IPC driver
 */
Int IPCManager_open(const char *name, int flags);

/*
 *  ======== IPCManager_close ========
 *      Close handle to the IPC driver
 */
Int IPCManager_close(int fd);

/*
 * ======== IPCManager_ioctl ========
 */
Int IPCManager_ioctl(int fd, UInt32 cmd, Ptr args);

/*
 * ======== IPCManager_fcntl========
 */
Int IPCManager_fcntl(int fd, int cmd, long arg);

/*
 * ======== IPCManager_getModuleStatus========
 *  This function converts the os specific (Linux) error code to
 *  module specific error code
 */
Int IPCManager_getModuleStatus(Int moduleId, Ptr args);

#endif
