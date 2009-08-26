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
 *  @file   HeapBufDrv.h
 *
 *  @brief      Declarations of OS-specific functionality for HeapBuf
 *
 *              This file contains declarations of OS-specific functions for
 *              HeapBuf.
 */


#ifndef HeapBufDrv_H_0xed17
#define HeapBufDrv_H_0xed17


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 *  See HeapBufDrvDefs.h
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the HeapBuf driver. */
Int HeapBufDrv_open (Void);

/* Function to close the HeapBuf driver. */
Int HeapBufDrv_close (Void);

/* Function to invoke the APIs through ioctl. */
Int HeapBufDrv_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* HeapBufDrv_H_0xed17 */
