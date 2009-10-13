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
 *  @file   SharedRegion.h
 *
 *  @brief      Shared Region Manager
 *
 *              The SharedRegion module is designed to be used in a
 *              multi-processor environment where there are memory regions that
 *              are shared and accessed across different processors. This module
 *              creates a shared memory region lookup table. This lookup table
 *              contains the processor's view for every shared region in the
 *              system. Each processor must have its own lookup table. Each
 *              processor's view of a particular shared memory region can be
 *              determined by the same table index across all lookup tables.
 *              Each table entry is a base and length pair. During runtime,
 *              this table along with the shared region pointer is used to do a
 *              quick address translation.<br>
 *              The shared region pointer (#SharedRegion_SRPtr) is a 32-bit
 *              portable pointer composed of an index and offset. The most
 *              significant bits of a #SharedRegion_SRPtr are used for the index.
 *              The index corresponds to the index of the entry in the lookup
 *              table. The offset is the offset from the base of the shared
 *              memory region. The maximum number of table entries in the lookup
 *              table will determine the number of bits to be used for the index.
 *              An increase in the index means the range of the offset would
 *              decrease. Here is sample code for getting a #SharedRegion_SRPtr
 *              and then getting a the real address pointer back.
 *              @Example
 *              @code
 *                  SharedRegion_SRPtr srptr;
 *                  UInt  index;
 *
 *                  // to get the index of the address
 *                  index = SharedRegion_getIndex(addr);
 *
 *                  // to get the shared region pointer for the address
 *                  srptr = SharedRegion_getSRPtr(addr, index);
 *
 *                  // to get the address back from the shared region pointer
 *                  addr = SharedRegion_getPtr(srptr);
 *              @endcode
 *
 *  ============================================================================
 */


#ifndef SHAREDREGION_H_0X5D8A
#define SHAREDREGION_H_0X5D8A


/* OSAL & Utils headers */
#include <Gate.h>
#include <Heap.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * macros & defines
 * =============================================================================
 */
/*!
 *  @def    SHAREDREGION_MODULEID
 *  @brief  Module ID for Shared region manager.
 */
#define SHAREDREGION_MODULEID      (0x5D8A)


/*!
 *  @def    SHAREDREGION_STATUSCODEBASE
 *  @brief  Error code base for Shared region manager.
 */
#define SHAREDREGION_STATUSCODEBASE      (SHAREDREGION_MODULEID << 12u)

/*!
 *  @def    SHAREDREGION_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define SHAREDREGION_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                                 + (SHAREDREGION_STATUSCODEBASE\
                                                 + (x))))

/*!
 *  @def    SHAREDREGION_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define SHAREDREGION_MAKE_SUCCESS(x)    (SHAREDREGION_STATUSCODEBASE + (x))

/*!
 *  @def    SHAREDREGION_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define SHAREDREGION_E_INVALIDARG       SHAREDREGION_MAKE_FAILURE(1)

/*!
 *  @def    SHAREDREGION_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define SHAREDREGION_E_MEMORY           SHAREDREGION_MAKE_FAILURE(2)

/*!
 *  @def    SHAREDREGION_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define SHAREDREGION_E_BUSY             SHAREDREGION_MAKE_FAILURE(3)

/*!
 *  @def    SHAREDREGION_E_FAIL
 *  @brief  Generic failure.
 */
#define SHAREDREGION_E_FAIL             SHAREDREGION_MAKE_FAILURE(4)

/*!
 *  @def    SHAREDREGION_E_NOTFOUND
 *  @brief  name not found in the SharedRegion.
 */
#define SHAREDREGION_E_NOTFOUND         SHAREDREGION_MAKE_FAILURE(4)

/*!
 *  @def    SHAREDREGION_E_ALREADYEXIST
 *  @brief  Entry already exists.
 */
#define SHAREDREGION_E_ALREADYEXIST     SHAREDREGION_MAKE_FAILURE(5)

/*!
 *  @def    SHAREDREGION_E_INVALIDSTATE
 *  @brief  Module is in invalid state.
 */
#define SHAREDREGION_E_INVALIDSTATE     SHAREDREGION_MAKE_FAILURE(6)

/*!
 *  @def    SHAREDREGION_E_OVERLAP
 *  @brief  Entries overlaps.
 */
#define SHAREDREGION_E_OVERLAP          SHAREDREGION_MAKE_FAILURE(7)

/*!
 *  @def    SHAREDREGION_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define SHAREDREGION_E_OSFAILURE        SHAREDREGION_MAKE_FAILURE(8)

/*!
 *  @def    SHAREDREGION_SUCCESS
 *  @brief  Operation successful.
 */
#define SHAREDREGION_SUCCESS            SHAREDREGION_MAKE_SUCCESS(0)

/*!
 *  @brief  Name of the reserved nameserver used for application.
 */
#define SHAREDREGION_NAMESERVER        "SHAREDREGION"

/*!
 *  @brief  Name of the reserved nameserver used for application.
 */
#define SHAREDREGION_INVALIDSRPTR      ((SharedRegion_SRPtr) 0xFFFFFFFF)

/*!
 *  @def    SHAREDREGION_S_ALREADYSETUP
 *  @brief  The GATEPETERSON module has already been setup in this process.
 */
#define SHAREDREGION_S_ALREADYSETUP     SHAREDREGION_MAKE_SUCCESS(1)


/* =============================================================================
 * Structure & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure for retrieving table entry information.
 */
typedef struct SharedRegion_Info {
    Bool   isValid;
    /*!< Specifies whether the table entry is valid or not. */
    Ptr    base;
    /*!< Pointer to the base address of table entry. */
    UInt32 len;
    /*!< The length of a table entry */
} SharedRegion_Info;


/*!
 *  @brief  Module configuration structure.
 */
typedef struct SharedRegion_Config {
    Gate_Handle gateHandle;
    /*!< Handle to a gate instance */
    Heap_Handle heapHandle;
    /*!< Handle to a heap instance */
    UInt32      maxRegions;
    /*!< Maximum number fo region, that can be created */
} SharedRegion_Config;


/* =============================================================================
 * Forward & Typedef
 * =============================================================================
 */
/*!
 *  @brief  Shared region pointer type
 */
typedef UInt32 * SharedRegion_SRPtr;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function to get the configuration */
Int32 SharedRegion_getConfig (SharedRegion_Config * config);

/* Function to setup the SharedRegion module. */
Int32 SharedRegion_setup (SharedRegion_Config * config);

/* Function to destroy the SharedRegion module. */
Int32 SharedRegion_destroy (void);

/* Fucntion to Add a memory segment to the lookup table during runtime by base
 * and length */
Int32 SharedRegion_add (UInt index, Ptr base, UInt32 len);

/* Returns the index for the specified address pointer */
Int32 SharedRegion_getIndex (Ptr addr);

/* Returns the address pointer associated with the shared region pointer */
Ptr SharedRegion_getPtr (SharedRegion_SRPtr srptr);

/* Returns the shared region pointer */
SharedRegion_SRPtr SharedRegion_getSRPtr (Ptr addr, Int index);

/* Gets the table entry information for the specified index and id */
void SharedRegion_getTableInfo (UInt                index,
                                UInt16              procId,
                                SharedRegion_Info * info);

/* Removes the memory segment at the specified index from the lookup table at
 * runtime */
Int32 SharedRegion_remove (UInt index);

/* Sets the base address of the entry in the table */
void SharedRegion_setTableInfo (UInt                index,
                                UInt16              procId,
                                SharedRegion_Info * info);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* SHAREDREGION_H_0X5D8A */
