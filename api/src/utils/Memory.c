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
/*============================================================================
 *  @file   Memory.c
 *
 *  @brief      Linux kernel Memory interface implementation.
 *
 *              This abstracts the Memory management interface in the kernel
 *              code. Allocation, Freeing-up, copy and address translate are
 *              supported for the kernel memory management.
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Memory.h>
#include <MemoryOS.h>
#include <Heap.h>
#include <Trace.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @brief      Allocates the specified number of bytes.
 *
 *  @param      ptr     Pointer where the size memory is allocated.
 *  @param      size    Amount of memory to be allocated.
 *  @param      align   Alignment constraints (power of 2)
 *
 *  @sa         Memory_calloc, MemoryOS_alloc
 */
Ptr
Memory_alloc (Heap_Handle heap, UInt32 size, UInt32 align)
{
    Ptr buffer = NULL;

    GT_3trace (curTrace, GT_ENTER, "Memory_alloc", heap, size, align);

    /* check whether the right paramaters are passed or not.*/
    GT_assert (curTrace, (size > 0));

    if (heap == NULL) {
        /* Call the kernel API for memory allocation */
        buffer = MemoryOS_alloc (size, align, 0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (buffer == NULL) {
            /*! @retval NULL Failed to allocate memory */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Memory_alloc",
                                 MEMORY_E_MEMORY,
                                 "Failed to allocate memory!");
        }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }
    else {
        buffer = Heap_alloc (heap, size, align);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (buffer == NULL) {
            /*! @retval NULL Heap_alloc failed */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Memory_alloc",
                                 MEMORY_E_MEMORY,
                                 "Heap_alloc failed!");
        }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "Memory_alloc", buffer);

    /*! @retval Pointer Success: Pointer to allocated buffer */
    return buffer;
}

/*!
 *  @brief      Allocates the specified number of bytes and memory is set to
 *              zero.
 *
 *  @param      ptr     Pointer where the size memory is allocated.
 *  @param      size    Amount of memory to be allocated.
 *  @param      align   Alignment constraints (power of 2)
 *
 *  @sa         Memory_alloc, MemoryOS_calloc
 */
Ptr
Memory_calloc (Heap_Handle heap, UInt32 size, UInt32 align)
{
    Ptr buffer = NULL;

    GT_3trace (curTrace, GT_ENTER, "Memory_calloc", heap, size, align);

    /* Check whether the right paramaters are passed or not.*/
    GT_assert (curTrace, (size > 0));

    if (heap == NULL) {
        buffer = MemoryOS_calloc (size, align, 0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (buffer == NULL) {
            /*! @retval NULL Failed to allocate memory */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Memory_calloc",
                                 MEMORY_E_MEMORY,
                                 "Failed to allocate memory!");
        }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }
    else {
        buffer = Heap_alloc (heap, size, align);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (buffer == NULL) {
            /*! @retval NULL Heap_alloc failed */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Memory_calloc",
                                 MEMORY_E_MEMORY,
                                 "Heap_alloc failed!");
        }
        else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
            buffer = Memory_set (buffer, 0, size);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (buffer == NULL) {
                /*! @retval NULL Memory_set to 0 failed */
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "Memory_calloc",
                                     MEMORY_E_MEMORY,
                                     "Memory_set to 0 failed!");
            }
        }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_0trace (curTrace, GT_LEAVE, "Memory_calloc");

    /*! @retval Pointer Success: Pointer to allocated buffer */
    return buffer;
}

/*!
 *  @brief      Frees up the specified chunk of memory.
 *
 *  @param      ptr     Pointer where the size memory is allocated.
 *  @param      size    Amount of memory to be allocated.
 *  @param      align   Alignment constraints (power of 2)
 *
 *  @sa         Memory_alloc, MemoryOS_calloc
 */
Void
Memory_free (Heap_Handle heap, Ptr ptr, UInt32 size)
{
    GT_2trace (curTrace, GT_ENTER, "Memory_free", heap, ptr);

    GT_assert (curTrace, (ptr != NULL));
    GT_assert (curTrace, (size > 0));

    if (heap == NULL) {
        MemoryOS_free (ptr, size, 0);
    }
    else {
        Heap_free (heap, ptr, size);
    }

    GT_0trace (curTrace, GT_LEAVE, "Memory_free");
}


/*!
 *  @brief      Function to translate an address.
 *
 *  @param      srcAddr  source address.
 *  @param      flags    Tranlation flags.
 */
Ptr
Memory_translate (Ptr srcAddr, Memory_XltFlags flags)
{
    Ptr buf = NULL;

    GT_2trace (curTrace, GT_ENTER, "Memory_tranlate", srcAddr, flags);
    
    buf = MemoryOS_translate (srcAddr, flags);

    GT_1trace (curTrace, GT_LEAVE, "Memory_tranlate", buf);

    /*! @retval Pointer Success: Pointer to updated destination buffer */
    return buf;
}

#if defined (__cplusplus)
}
#endif /* defined (_cplusplus)*/
