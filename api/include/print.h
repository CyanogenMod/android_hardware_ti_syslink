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
 *  @file   print.h
 *
 *  @desc   Interface declaration of OS printf abstraction.
 *
 *  ============================================================================
 */


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*  ----------------------------------- IPC headers */
#include <ipc.h>
#include <_ipc.h>


/** ============================================================================
 *  @func   PRINT_init
 *
 *  @desc   Initializes the PRINT sub-component.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */

DSP_STATUS
PRINT_init (Void) ;

/** ============================================================================
 *  @deprecated The deprecated function PRINT_Initialize has been replaced
 *              with PRINT_init.
 *  ============================================================================
 */
#define PRINT_Initialize PRINT_init


/** ============================================================================
 *  @func   PRINT_exit
 *
 *  @desc   Releases resources used by this sub-component.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */

DSP_STATUS
PRINT_exit (Void) ;

/** ============================================================================
 *  @deprecated The deprecated function PRINT_Finalize has been replaced
 *              with PRINT_exit.
 *  ============================================================================
 */
#define PRINT_Finalize PRINT_exit


/** ============================================================================
 *  @func   PRINT_Printf
 *
 *  @desc   Provides standard printf functionality abstraction.
 *
 *  @arg    format
 *              Format string.
 *  @arg    ...
 *              Variable argument list.
 *
 *  @ret    None
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
#if defined (TRACE_KERNEL)

Void
PRINT_Printf (Pstr format, ...) ;
#endif

#if defined (TRACE_USER)
/*  ----------------------------------------------------------------------------
 *  Extern declaration for printf to avoid compiler warning.
 *  ----------------------------------------------------------------------------
 */
extern int printf (const char * format, ...) ;

#define PRINT_Printf printf
#endif


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
