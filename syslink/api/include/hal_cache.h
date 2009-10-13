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
 *  @file   hal_cache.h
 *
 *
 *  @desc   This file declares cache functions of HAL (Hardware Abstraction
 *          Layer)
 *  ============================================================================
 */


#if !defined (HAL_CACHE_H)
#define HAL_CACHE_H


/*  ----------------------------------- CGTOOLS headers             */
#include <stddef.h>

/*  ----------------------------------- DSP/BIOS LINK headers       */
//#include <_hal_cache.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @func   HAL_cacheInv
 *
 *  @desc   Invalidate cache contents for specified memory region.
 *
 *  @arg    addr
 *              Address of memory region for which cache is to be invalidated.
 *  @arg    size
 *              Size of memory region for which cache is to be invalidated.
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
inline Void HAL_cacheInv (Ptr addr, size_t size)
{
    //HAL_CACHE_INV (addr, size) ;
}


/** ============================================================================
 *  @func   HAL_cacheWb
 *
 *  @desc   Write-back cache contents for specified memory region.
 *
 *  @arg    addr
 *              Address of memory region for which cache is to be invalidated.
 *  @arg    size
 *              Size of memory region for which cache is to be invalidated.
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
inline Void HAL_cacheWb (Ptr addr, size_t size)
{
    ;
}


/** ============================================================================
 *  @func   HAL_cacheWbInv
 *
 *  @desc   Write-back and invalidate cache contents for specified memory region
 *
 *  @arg    addr
 *              Address of memory region for which cache is to be invalidated.
 *  @arg    size
 *              Size of memory region for which cache is to be invalidated.
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
inline Void HAL_cacheWbInv (Ptr addr, size_t size)
{
    //HAL_CACHE_WBINV (addr, size) ;
}


/** ============================================================================
 *  @func   HAL_cacheWbAll
 *
 *  @desc   Write-back complete cache contents.
 *
 *  @arg    None
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
inline Void HAL_cacheWbAll ()
{
    //HAL_CACHE_WBALL ;
}


/** ============================================================================
 *  @func   HAL_cacheWbInvAll
 *
 *  @desc   Write-back and invaliate complete cache contents.
 *
 *  @arg    None
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
inline Void HAL_cacheWbInvAll ()
{
    //HAL_CACHE_WBINVALL ;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (HAL_CACHE_H) */
