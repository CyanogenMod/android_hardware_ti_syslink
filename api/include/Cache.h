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
 *  @file   Cache.h
 *
 *  @brief      Defines Cache API interface.
 *  ============================================================================
 */


#ifndef CACHE_H
#define CACHE_H


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*
 *  ======== Cache_inv ========
 */
Void Cache_inv(Ptr blockPtr, UInt32 byteCnt, Bool wait) ;

/*
 *  ======== Cache_wb ========
 */
Void Cache_wb(Ptr blockPtr, UInt32 byteCnt, Bool wait) ;

/*
 *  ======== Cache_wbInv ========
 */
Void Cache_wbInv(Ptr blockPtr, UInt32 byteCnt, Bool wait) ;

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* CACHE_H */
