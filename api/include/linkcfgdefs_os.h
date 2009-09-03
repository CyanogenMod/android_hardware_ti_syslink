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
 *  @file   linkcfgdefs_os.h
 *
 *  @desc   Defines OS-specific constants and interfaces for configuration of
 *          DSP/BIOS LINK.
 *  ============================================================================
 */


#if !defined (LINKCFGDEFS_OS_H)
#define LINKCFGDEFS_OS_H


/*  ----------------------------------- DSP/BIOS LINK Headers       */
#include <dsplink.h>


#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @name   LINKCFG_GppOs
 *
 *  @desc   This structure defines the configuration structure for the GPP OS
 *          specific configuration.
 *
 *  @field  handleSignals
 *              Indicates whether signals should be handled for cleanup on
 *              abnormal process termination.
 *  @field  numSignals
 *              Indicates the number of signals to be handled.
 *  @field  sigNumArray
 *              Pointer to the array of signals to be handled.
 *  ============================================================================
 */
typedef struct LINKCFG_GppOs_tag {
    Bool               handleSignals ;
    Uint32             numSignals ;
    Uint32 *           sigNumArray ;
} LINKCFG_GppOs ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (LINKCFGDEFS_OS_H) */
