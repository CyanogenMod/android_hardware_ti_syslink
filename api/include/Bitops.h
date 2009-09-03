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
 *  @file   Bitops.h
 *
 *  @brief      Defines for bit operations macros.
 *  ============================================================================
 */


#ifndef UTILS_BITOPS_H_0X838E
#define UTILS_BITOPS_H_0X838E


#ifdef SYSLINK_BUILDOS_LINUX
#include <Atomic_Ops.h>
#endif

#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @brief  Sets bit at given position.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 */
#define SET_BIT(num,pos)            ((num) |= (1u << (pos)))

/*!
 *  @brief  Clears bit at given position.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 */
#define CLEAR_BIT(num,pos)          ((num) &= ~(1u << (pos)))

/*!
 *  @brief  Tests if bit at given position is set.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 */
#define TEST_BIT(num,pos)           ((((num) & (1u << (pos))) >> (pos)) & 0x01)


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* UTILS_BITOPS_H_0X838E */
