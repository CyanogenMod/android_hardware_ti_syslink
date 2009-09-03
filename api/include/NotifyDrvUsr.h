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
 *  @file       NotifyDrvUsr.h
 *
 *  @brief      Declarations of OS-specific functionality for Notify
 *
 *              This file contains declarations of OS-specific functions for
 *              Notify.
 *  ============================================================================
 */



#ifndef NotifyDrvUsr_H_0x5f84
#define NotifyDrvUsr_H_0x5f84


/* Standard headers */
#include <Std.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 *  See NotifyDrvDefs.h
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the Notify driver. */
Int NotifyDrvUsr_open (Bool createThread);

/* Function to close the Notify driver. */
Int NotifyDrvUsr_close (Bool deleteThread);

/* Function to invoke the APIs through ioctl. */
Int NotifyDrvUsr_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* NotifyDrvUsr_H_0x5f84 */
