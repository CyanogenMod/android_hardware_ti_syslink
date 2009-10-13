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
 *  @file   ListMPSharedMemoryDrv.h
 *
 *  @brief      Declarations of OS-specific functionality for ListMPSharedMemory
 *
 *              This file contains declarations of OS-specific functions for
 *              ListMPSharedMemory.
 *
 *  ============================================================================
 */


#ifndef ListMPSharedMemoryDrv_H_0xdf43
#define ListMPSharedMemoryDrv_H_0xdf43


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 *  See ListMPSharedMemoryDrvDefs.h
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the ListMPSharedMemory driver. */
Int ListMPSharedMemoryDrv_open (Void);

/* Function to close the ListMPSharedMemory driver. */
Int ListMPSharedMemoryDrv_close (Void);

/* Function to invoke the APIs through ioctl. */
Int ListMPSharedMemoryDrv_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* ListMPSharedMemoryDrv_H_0xdf43 */
