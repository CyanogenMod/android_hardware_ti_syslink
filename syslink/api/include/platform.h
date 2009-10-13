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
 *  @file   platform.h
 *
 *  @desc   Defines platform specific attributes.
 *  ============================================================================
 */


#if !defined (PLATFORM_H)
#define PLATFORM_H


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @const  MAX_PROCESSORS
 *
 *  @desc   Maximum number of processors supported for this platform.
 *  ============================================================================
 */
#define MAX_PROCESSORS        2

/** ============================================================================
 *  @const  MAX_DSPS
 *
 *  @desc   Maximum number of DSPs supported for this platform.
 *  ============================================================================
 */
#define MAX_DSPS              (MAX_PROCESSORS - 1)

/** ============================================================================
 *  @const  ID_GPP
 *
 *  @desc   Processor Identifier for the GPP for this platform.
 *  ============================================================================
 */
#define ID_GPP                1


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* if !defined (PLATFORM_H) */
