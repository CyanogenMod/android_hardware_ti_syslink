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
 *  @file   Memory.h
 *
 *  @brief      Kernel utils Memory interface definitions.
 *
 *              This abstracts the Memory management interface in the kernel
 *              code. Allocation, Freeing-up, copy and address translate are
 *              supported for the kernel memory management.
 *  ============================================================================
 */


#ifndef MEMORY_H_0xC97E
#define MEMORY_H_0xC97E


/* OSAL and utils */
#include <MemoryDefs.h>
#include <MemoryOS.h>

/* Module headers */
#include <Heap.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    MEMORY_MODULEID
 *  @brief  Module ID for Memory OSAL module.
 */
#define MEMORY_MODULEID                 (UInt16) 0xC97E

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
* @def   MEMORY_STATUSCODEBASE
* @brief Stauts code base for MEMORY module.
*/
#define MEMORY_STATUSCODEBASE            (MEMORY_MODULEID << 12u)

/*!
* @def   MEMORY_MAKE_FAILURE
* @brief Convert to failure code.
*/
#define MEMORY_MAKE_FAILURE(x)          ((Int) (0x80000000  \
                                         + (MEMORY_STATUSCODEBASE + (x))))
/*!
* @def   MEMORY_MAKE_SUCCESS
* @brief Convert to success code.
*/
#define MEMORY_MAKE_SUCCESS(x)            (MEMORY_STATUSCODEBASE + (x))

/*!
* @def   MEMORY_E_MEMORY
* @brief Indicates Memory alloc/free failure.
*/
#define MEMORY_E_MEMORY                   MEMORY_MAKE_FAILURE(1)

/*!
* @def   MEMORY_E_INVALIDARG
* @brief Invalid argument provided
*/
#define MEMORY_E_INVALIDARG               MEMORY_MAKE_FAILURE(2)

/*!
* @def   MEMORY_SUCCESS
* @brief Operation successfully completed
*/
#define MEMORY_SUCCESS                    MEMORY_MAKE_SUCCESS(0)

/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* TBD: Is init/exit needed? */

/* Allocates the specified number of bytes. */
Ptr Memory_alloc (Heap_Handle heap, UInt32 size, UInt32 align);

/* Allocates the specified number of bytes and memory is set to zero. */
Ptr Memory_calloc (Heap_Handle heap, UInt32 size, UInt32 align);

/* Frees up the specified chunk of memory. */
Void Memory_free (Heap_Handle heap, Ptr ptr, UInt32 size);

/* Function to translate an address. */
Ptr Memory_translate (Ptr srcAddr, Memory_XltFlags flags);

/* Add other Memory module APIs */

/* =============================================================================
 *  APIs that are added for MemoryOS
 * =============================================================================
 */
#define Memory_map           MemoryOS_map
#define Memory_unmap         MemoryOS_unmap
#define Memory_copy          MemoryOS_copy
#define Memory_set           MemoryOS_set


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef MEMORY_H_0xC97E */
