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
 *  @file   osal.h
 *
 *  @desc   Defines the interfaces for initializing and finalizing OSAL.
 *  ============================================================================
 */


#if !defined (OSAL_H)
#define OSAL_H


/*  ----------------------------------- IPC headers */
#include <ipc.h>
#include <_ipc.h>

/*  ----------------------------------- OSAL Headers                */
#include <isr.h>
#include <mem_os.h>
#include <mem.h>
#include <dpc.h>
#include <sync.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @func   OSAL_init
 *
 *  @desc   Initializes the OS Adaptation layer.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EMEMORY
 *              Out of memory error.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    OSAL_exit
 *  ============================================================================
 */
EXPORT_API
DSP_STATUS
OSAL_init (Void) ;

/** ============================================================================
 *  @deprecated The deprecated function OSAL_Initialize has been replaced
 *              with OSAL_init.
 *  ============================================================================
 */
#define OSAL_Initialize OSAL_init


/** ============================================================================
 *  @func   OSAL_exit
 *
 *  @desc   Releases OS adaptation layer resources indicating that they would
 *          no longer be used.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EMEMORY
 *              Out of memory error.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  Subcomponent must be initialized.
 *
 *  @leave  None
 *
 *  @see    OSAL_init
 *  ============================================================================
 */
EXPORT_API
DSP_STATUS
OSAL_exit (Void) ;

/** ============================================================================
 *  @deprecated The deprecated function OSAL_Finalize has been replaced
 *              with OSAL_exit.
 *  ============================================================================
 */
#define OSAL_Finalize OSAL_exit


#if defined (__cplusplus)
}
#endif


#endif /* !defined (OSAL_H) */
