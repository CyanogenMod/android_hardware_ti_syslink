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
 *  @file   _HeapBuf.h
 *
 *  @brief      Internal definitions  for HeapBuf based memory allocator
 *              internal structure definitions.
 *
 *  ============================================================================
 */


#ifndef _HEAPBUF_H_0xdf74
#define _HEAPBUF_H_0xdf74


#if defined (__cplusplus)
extern "C" {
#endif

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining attribute parameters for the Heap Buf module.
 */
typedef struct HeapBuf_Attrs_tag {
    volatile Bits32 version;
    /*!< Version of module */
    volatile Bits32 status;
    /*!< Status of module */
    volatile Bits32 numFreeBlocks;
    /*!< Number of free blocks */
    volatile Bits32 minFreeBlocks;
    /*!< Minimum free blocks */
    volatile Bits32 blockSize;
    /*!< Block Size */
    volatile Bits32 align;
    /*!< Align */
    volatile Bits32 numBlocks;
    /*!< Number of Blocks */
    volatile Bits32 bufSize;
    /*!< size of Buffers to be allocated */
    volatile Bits32 buf;
    /*!< Buffer */
} HeapBuf_Attrs;

/*!
 *  @brief  Structure defining processor related information for the
 *          Heap Buf module.
 */
typedef struct HeapBuf_ProcAttrs_tag {
    Bool   creator;   /*!< Creator or opener */
    UInt16 procId;    /*!< Processor Identifier */
    UInt32 openCount; /*!< How many times it is opened on a processor */
} HeapBuf_ProcAttrs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* _HEAPBUF_H_0xdf74 */

