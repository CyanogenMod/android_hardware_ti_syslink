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
 *  @file   SysMemMgrDrv.h
 *
 *  @brief      Declarations of OS-specific functionality for SysMgr
 *
 *              This file contains declarations of OS-specific functions for
 *              ProcMgr.
 *  ============================================================================
 */


#ifndef SYSTEMMEMORYMANAGERDRV_H_0xb53d
#define SYSTEMMEMORYMANAGERDRV_H_0xb53d


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 *  See ProcMgrDrvDefs.h
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the ProcMgr driver. */
Int SysMemMgrDrv_open (Void);

/* Function to close the ProcMgr driver. */
Int SysMemMgrDrv_close (Void);

/* Function to invoke the APIs through ioctl. */
Int SysMemMgrDrv_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* SYSTEMMEMORYMANAGERDRV_H_0xb53d */
