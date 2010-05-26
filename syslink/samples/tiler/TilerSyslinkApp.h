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
 *  @file   TilerSysLinkApp.h
 *
 *
 *
 *  ============================================================================
 */


#ifndef TILERSYSLINKTESTAPP_0xf2ba
#define TILERSYSLINKTESTAPP_0xf2ba



#if defined (__cplusplus)
extern "C" {
#endif

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */


/* =============================================================================
 *  APIs
 * =============================================================================
 */

/*!
 *  @brief  Function to Test use buffer functionality using Tiler and Syslink
 *          IPC
 */
Int SyslinkUseBufferTest(Int, Bool, UInt);

/*!
 *  @brief  Function to Test Physical to Virtual address translation for tiler
 *          address
 */
Int SyslinkVirtToPhysTest(void);

/*!
 *  @brief  Function to Test Physical to Virtual pages address translation for
  *         tiler address
 */
Int SyslinkVirtToPhysPagesTest(void);

/*!
 *  @brief  Function to Test repeated mapping and unmapping of addresses to
 *          Ducati virtual space
 */
Int SyslinkMapUnMapTest(UInt);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif
