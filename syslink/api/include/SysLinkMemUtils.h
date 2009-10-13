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
 *  @file   SysLinkMemUtils.h
 *
 *
 *
 *  ============================================================================
 */


#ifndef SYSLINKMEMUTILS_H_0xf2ba
#define SYSLINKMEMUTILS_H_0xf2ba


/* Standard headers */
#include <Std.h>
#include <ProcMgr.h>


#if defined (__cplusplus)
extern "C" {
#endif

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining the MPU address to map to Remote Address
 */
typedef struct {
    UInt32 mpuAddr;
    /*!< Host Address to Map*/
    UInt32 size;
    /*!< Size of the Buffer to Map */
} SyslinkMemUtils_MpuAddrToMap;

/* =============================================================================
 *  APIs
 * =============================================================================
 */

Int
SysLinkMemUtils_map (SyslinkMemUtils_MpuAddrToMap mpuAddrList[],
                     UInt32                       numOfBuffers,
                     UInt32 *                     mappedAddr,
                     ProcMgr_MapType              memType,
                     ProcMgr_ProcId               procId);

Int
SysLinkMemUtils_unmap (UInt32 mappedAddr, ProcMgr_ProcId procId);

Int
SysLinkMemUtils_virtToPhysPages (UInt32 remoteAddr,
                                 UInt32 numOfPages,
                                 UInt32 physEntries[],
                                 ProcMgr_ProcId procId);

Int
SysLinkMemUtils_virtToPhys (UInt32          remoteAddr,
                            UInt32 *        physAddr,
                            ProcMgr_ProcId  procId);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* SYSLINKMEMUTILS_H_0xf2ba */
