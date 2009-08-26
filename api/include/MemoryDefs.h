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
 *  @file   MemoryDefs.h
 *
 *  @brief      Definitions for Memory module.
 *
 *              This provides macros and type definitions for the Memory module.
 *
 *  ============================================================================
 */


#ifndef MEMORYDEFS_H_0x73E4
#define MEMORYDEFS_H_0x73E4


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief   Enumerates the types of Caching for memory regions
 */
typedef enum {
    MemoryOS_CacheFlags_Default           = 0x00000000,
    /*!< Default flags - Cached */
    MemoryOS_CacheFlags_Cached            = 0x00010000,
    /*!< Cached memory */
    MemoryOS_CacheFlags_Uncached          = 0x00020000,
    /*!< Uncached memory */
    MemoryOS_CacheFlags_EndValue          = 0x00030000
    /*!< End delimiter indicating start of invalid values for this enum */
} MemoryOS_CacheFlags;

/*!
 *  @brief   Enumerates the types of memory allocation
 */
typedef enum {
    MemoryOS_MemTypeFlags_Default         = 0x00000000,
    /*!< Default flags - virtually contiguous */
    MemoryOS_MemTypeFlags_Physical        = 0x00000001,
    /*!< Physically contiguous */
    MemoryOS_MemTypeFlags_Dma             = 0x00000002,
    /*!< Physically contiguous */
    MemoryOS_MemTypeFlags_EndValue        = 0x00000003
    /*!< End delimiter indicating start of invalid values for this enum */
} MemoryOS_MemTypeFlags;

/*!
 *  @brief   Enumerates the types of translation
 */
typedef enum {
    Memory_XltFlags_Virt2Phys       = 0x00000000,
    /*!< Virtual to physical */
    Memory_XltFlags_Phys2Virt       = 0x00000001,
    /*!< Virtual to physical */
    Memory_XltFlags_EndValue        = 0x00000002
    /*!< End delimiter indicating start of invalid values for this enum */
} Memory_XltFlags;


/*!
 *  @brief   Structure containing information required for mapping a
 *           memory region.
 */
typedef struct MemoryOS_MapInfo_tag {
    UInt32   src;
    /*!< Address to be mapped. */
    UInt32   size;
    /*!< Size of memory region to be mapped. */
    UInt32   dst;
    /*!< Mapped address. */
    Bool     isCached;
    /*!< Whether the mapping is to a cached area or uncached area. */
    Ptr      drvHandle;
    /*!< Handle to the driver that is implementing the mmap call. Ignored for
         Kernel-side version. */
} MemoryOS_MapInfo ;

/*!
 *  @brief   Structure containing information required for unmapping a
 *           memory region.
 */
typedef struct MemoryOS_UnmapInfo_tag {
    UInt32  addr;
    /*!< Address to be unmapped.*/
    UInt32  size;
    /*!< Size of memory region to be unmapped.*/
    Bool    isCached;
    /*!< Whether the mapping is to a cached area or uncached area.  */
} MemoryOS_UnmapInfo;

/*!
 *  @brief   Structure containing information required for mapping a
 *           memory region.
 */
#define Memory_MapInfo   MemoryOS_MapInfo

/*!
 *  @brief   Structure containing information required for unmapping a
 *           memory region.
 */
#define Memory_UnmapInfo MemoryOS_UnmapInfo


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef MEMORYDEFS_H_0x73E4 */
