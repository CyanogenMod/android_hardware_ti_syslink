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
 *  @file   omap4430procDrvUsr.h
 *
 *  @brief      Declarations of OS-specific functionality for OMAP4430PROC
 *  ============================================================================
 */


#ifndef omap4430procDrvUsr_H_0xbbec
#define omap4430procDrvUsr_H_0xbbec


/* Standard headers */
#include <Std.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 *  See OMAP4430PROCDrvDefs.h
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the OMAP4430PROC driver. */
Int OMAP4430PROCDrvUsr_open (Void);

/* Function to close the OMAP4430PROC driver. */
Int OMAP4430PROCDrvUsr_close (Void);

/* Function to invoke the APIs through ioctl. */
Int OMAP4430PROCDrvUsr_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* omap4430procDrvUsr_H_0xbbec */
