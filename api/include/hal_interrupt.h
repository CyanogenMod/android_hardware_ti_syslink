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
 *  @file   hal_intgen.h
 *
 *  @desc   Hardware Abstraction Layer for OMAP.
 *          Declares necessary functions for Interrupt Handling.
 *  ============================================================================
 */


#if !defined (HAL_INTERRUPT_H)
#define HAL_INTERRUPT_H


/*  ----------------------------------- DSP/BIOS Link               */
#include <ipc.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @func   HAL_InterruptEnable
 *
 *  @desc   Enable the mailbox interrupt
 *
 *  @arg    intId
 *              ID of the interrupt to be enabled
 *
 *  @ret    None
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None.
 *  ============================================================================
 */
NORMAL_API
Void
HAL_InterruptEnable (IN Uint32 intId) ;


/** ============================================================================
 *  @func   HAL_InterruptDisable
 *
 *  @desc   Disable the mailbox interrupt
 *
 *  @arg    intId
 *              ID of the interrupt to be disabled
 *
 *  @ret    None
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None.
 *  ============================================================================
 */
NORMAL_API
Void
HAL_InterruptDisable (IN Uint32 intId) ;


/** ============================================================================
 *  @func   HAL_WaitClearInterrupt
 *
 *  @desc   Wait for interrupt to be cleared.
 *
 *  @arg    intId
 *              ID of the interrupt to wait to be cleared.
 *
 *  @ret    DSP_SOK
 *              On success
 *          DSP_ETIMEOUT
 *              Timeout while interrupting the DSP
 *          DSP_EFAIL
 *              General failure
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
HAL_WaitClearInterrupt (IN Uint32 intId) ;


/** ============================================================================
 *  @func   HAL_InterruptDsp
 *
 *  @desc   Sends a specified interrupt to the DSP.
 *
 *  @arg    intId
 *              ID of the interrupt to be sent.
 *  @arg    value
 *              Value to be sent with the interrupt.
 *
 *  @ret    DSP_SOK
 *              On success
 *          DSP_ETIMEOUT
 *              Timeout while interrupting the DSP
 *          DSP_EFAIL
 *              General failure
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
HAL_InterruptDsp (IN Uint32 intId, Uint32 value) ;


/** ============================================================================
 *  @func   HAL_ClearDspInterrupt
 *
 *  @desc   Clears the specified DSP interrupt.
 *
 *  @arg    intId
 *              ID of the interrupt to be cleared.
 *
 *  @ret    value
 *              Value returned with the interrupt.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None.
 *  ============================================================================
 */
NORMAL_API
Uint32
HAL_ClearDspInterrupt (IN Uint32 intId) ;


#if defined (__cplusplus)
}
#endif


#endif  /* !defined (HAL_INTERRUPT_H) */
