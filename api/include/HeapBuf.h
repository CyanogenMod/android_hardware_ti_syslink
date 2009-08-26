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
 *  @file   HeapBuf.h
 *
 *  @brief      Defines HeapBuf based memory allocator.
 *
 *  Heap implementation that manages fixed size buffers that can be used
 *  in a multiprocessor system with shared memory.<br>
 *  <br>
 *  The HeapBuf manager provides functions to allocate and free storage from a
 *  heap of type HeapBuf which inherits from Heap. HeapBuf manages a single
 *  fixed-size buffer, split into equally sized allocable blocks.<br>
 *  <br>
 *  The HeapBuf manager is intended as a very fast memory
 *  manager which can only allocate blocks of a single size. It is ideal for
 *  managing a heap that is only used for allocating a single type of object,
 *  or for objects that have very similar sizes.<br>
 *  <br>
 *  This module is instance based. Each instance requires shared memory
 *  (for the buffers and other internal state).  This is specified via the
 *  sharedAddr parameter to the create. The proper sharedAddrSize parameter
 *  can be determined via the #HeapBuf_sharedMemReq call. Note: the parameters
 *  to this function must be the same that will used to create the instance.<br>
 *  <br>
 *  The HeapBuf module uses a instance to store instance information
 *  when an instance is created and the name parameter is non-NULL.
 *  If a name is supplied, it must be unique for all HeapBuf instances.<br>
 *  <br>
 *  The create initializes the shared memory as needed. The shared memory must
 *  be initialized to 0 before the HeapBuf instance is created or
 *  opened.<br>
 *  <br>
 *  Once an instance is created, an open can be performed. The
 *  open is used to gain access to the same HeapBuf instance.
 *  Generally an instance is created on one processor and opened on the
 *  other processor(s).<br>
 *  <br>
 *  The open returns a HeapBuf instance handle like the create,
 *  however the open does not modify the shared memory.<br>
 *  <br>
 *  There are two options when opening the instance:<br>
 *  -Supply the same name as specified in the create. The HeapBuf module
 *  queries the NameServer to get the needed information.<br>
 *  -Supply the same sharedAddr value as specified in the create.<br>
 *  <br>
 *  If the open is called before the instance is created, open returns NULL.<br>
 *  <br>
 *  Constraints:<br>
 *  -Align parameter must be a power of 2.<br>
 *  -The buffer passed to dynamically create a HeapBuf must be aligned
 *   according to the alignment parameter, and must be large enough to account
 *   for the actual block size after it has been rounded up to a multiple of
 *   the alignment.
 *
 *  ============================================================================
 */


#ifndef HEAPBUF_H_0x4CD5
#define HEAPBUF_H_0x4CD5

/* Osal & Utils headers */
#include <Heap.h>
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    HEAPBUF_MODULEID
 *  @brief  Unique module ID.
 */
#define HEAPBUF_MODULEID        (0x4CD5)

/* =============================================================================
 * Module Success and Failure codes
 * =============================================================================
 */
/*!
 *  @def    HEAPBUF_STATUSCODEBASE
 *  @brief  Error code base for HeapBuf.
 */
#define HEAPBUF_STATUSCODEBASE  (HEAPBUF_MODULEID << 12u)

/*!
 *  @def    HEAPBUF_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define HEAPBUF_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                               + (HEAPBUF_STATUSCODEBASE  \
                                               + (x))))

/*!
 *  @def    HEAPBUF_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define HEAPBUF_MAKE_SUCCESS(x)    (HEAPBUF_STATUSCODEBASE + (x))

/*!
 *  @def    HEAPBUF_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define HEAPBUF_E_INVALIDARG       HEAPBUF_MAKE_FAILURE(1)

/*!
 *  @def    HEAPBUF_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define HEAPBUF_E_MEMORY           HEAPBUF_MAKE_FAILURE(2)

/*!
 *  @def    HEAPBUF_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define HEAPBUF_E_BUSY             HEAPBUF_MAKE_FAILURE(3)

/*!
 *  @def    HEAPBUF_E_FAIL
 *  @brief  Generic failure.
 */
#define HEAPBUF_E_FAIL             HEAPBUF_MAKE_FAILURE(4)

/*!
 *  @def    HEAPBUF_E_NOTFOUND
 *  @brief  Name not found in the nameserver.
 */
#define HEAPBUF_E_NOTFOUND         HEAPBUF_MAKE_FAILURE(5)

/*!
 *  @def    HEAPBUF_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define HEAPBUF_E_INVALIDSTATE     HEAPBUF_MAKE_FAILURE(6)

/*!
 *  @def    HEAPBUF_E_NOTONWER
 *  @brief  Instance is not created on this processor.
 */
#define HEAPBUF_E_NOTONWER         HEAPBUF_MAKE_FAILURE(7)

/*!
 *  @def    HEAPBUF_E_REMOTEACTIVE
 *  @brief  Remote opener of the instance has not closed the instance.
 */
#define HEAPBUF_E_REMOTEACTIVE     HEAPBUF_MAKE_FAILURE(8)

/*!
 *  @def    HEAPBUF_E_INUSE
 *  @brief  Indicates that the instance is in use..
 */
#define HEAPBUF_E_INUSE            HEAPBUF_MAKE_FAILURE(9)

/*!
 *  @def    HEAPBUF_E_BUFALIGN
 *  @brief  Indicates that the buffer is not properly aligned.
 */
#define HEAPBUF_E_BUFALIGN         HEAPBUF_MAKE_FAILURE(10)

/*!
 *  @def    HEAPBUF_E_INVALIDALIGN
 *  @brief  Indicates that the buffer alignment parameter is not a
 *          power of 2.
 */
#define HEAPBUF_E_INVALIDALIGN     HEAPBUF_MAKE_FAILURE(11)

/*!
 *  @def    HEAPBUF_E_INVALIDBUFSIZE
 *  @brief  Indicates that the buffer size is invalid (too small)
 */
#define HEAPBUF_E_INVALIDBUFSIZE   HEAPBUF_MAKE_FAILURE(12)

/*!
 *  @def    HEAPBUF_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define HEAPBUF_E_OSFAILURE        HEAPBUF_MAKE_FAILURE(13)

/*!
 *  @def    HEAPBUF_E_VERSION
 *  @brief  Version mismatch error.
 */
#define HEAPBUF_E_VERSION          HEAPBUF_MAKE_FAILURE(14)

/*!
 *  @def    HEAPBUF_SUCCESS
 *  @brief  Operation successful.
 */
#define HEAPBUF_SUCCESS            HEAPBUF_MAKE_SUCCESS(0)

/*!
 *  @def    HEAPBUF_S_ALREADYSETUP
 *  @brief  The HEAPBUF module has already been setup in this process.
 */
#define HEAPBUF_S_ALREADYSETUP     HEAPBUF_MAKE_SUCCESS(1)

/*!
 *  @def    HEAPBUF_S_SETUP
 *  @brief  Other HeapBuf clients have still setup the HeapBuf module.
 */
#define HEAPBUF_S_SETUP            HEAPBUF_MAKE_SUCCESS(2)


/* =============================================================================
 * Macros
 * =============================================================================
 */
/*!
 *  @def    HEAPBUF_CREATED
 *  @brief  Creation of Heap Buf succesful.
*/
#define HEAPBUF_CREATED            (0x05251995u)

/*!
 *  @def    HEAPBUF_VERSION
 *  @brief  Version.
 */
#define HEAPBUF_VERSION            (1u)


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining config parameters for the HeapBuf module.
 */
typedef struct HeapBuf_Config_tag {
    UInt32      maxNameLen;
    /*!< Maximum length of name */
    Bool        useNameServer;
    /*!< Whether to have this module use the NameServer or not. If the
     *   NameServer is not needed, set this configuration parameter to false.
     *   This informs HeapBuf not to pull in the NameServer module. In this
     *   case, all names passed into create and open are ignored.
     */
    Bool        trackMaxAllocs;
    /*!<
     *  Track the maximum number of allocated blocks
     *
     *  This will enable/disable the tracking of the maximum number of
     *  allocations for a HeapBuf instance.  This maximum refers to the "all
     *  time" maximum number of allocations for the history of a HeapBuf
     *  instance, not the current number of allocations.
     *
     *  Tracking the maximum might incur a performance hit when allocating
     *  and/or freeing. If this feature is not needed, setting this to false
     *  avoids the performance penalty.
     *
     */
} HeapBuf_Config;

/*!
 *  @brief  Structure defining parameters for the Heap Buf module.
 */
typedef struct HeapBuf_Params_tag {
    Gate_Handle gate;
    /*!< Lock used for critical region management of the buffer */

    Bool exact;
    /*!<
     *  Only allocate if the requested size is an exact match
     */

    String name;
    /*!<
     *  If using NameServer, name of this instance.
     *
     *  The name (if not NULL) must be unique among all HeapBuf
     *  instances in the entire system.
     */

    Int resourceId;
    /*!
     *  resourceId specifies the resource id of the hardware linked list
     *
     *  The ListMP module supports fast linked list (hardware aided by
     *  linked lists). This parameter allows HeapBuf to use one of the
     *  hardware linked list to use.
     *
     *  Not all devices supports the a hardware linked list. In this
     *  case, HeapBuf uses the {@link #gate} provided and shared memory.
     *
     *  The default of -1 denotes not to use the a hardware linked list.
     */

    Bool cacheFlag;
    /*!<
     *  cache flag used to denote whether to perform cache coherency calls
     *
     *  HeapBuf by default performs cache coherency calls on the shared
     *  memory. If there is no cache enabled or if there is a unicache,
     *  a HeapBuf instance can be configured not to call the cache
     *  coherency calls.
     */

    UInt32 align;
    /*!<
     *  Alignment (in MAUs) of each block.
     *
     *  The alignment must be a power of 2. If the value 0 is specified,
     *  the value will be changed to meet the minimum structure alignment
     *  requirements so the actual alignment may be larger.
     *
     *  For static creates, the HeapBuf will allocate space for the buffer
     *  and will align the buffer on this requested alignment.
     *
     *  For dynamic creates, this parameter is used solely for error checking.
     *  The buffer provided to a dynamically created HeapBuf must be aligned
     *  and an assert will be raised if the buffer is not properly
     *  aligned. For dynamic creates, HeapBuf will NOT adjust the buffer to
     *  satisfy the alignment.
     *
     *  The default alignment is 0.
     */

    UInt numBlocks;
    /*!<
     *  Number of fixed-size blocks.
     *
     *  Required parameter.
     *
     *  The default number of blocks is 0.
     */

    UInt32 blockSize;
    /*!<
     *  Size (in MAUs) of each block.
     *
     *  HeapBuf will round the blockSize up to the nearest multiple of the
     *  alignment, so the actual blockSize may be larger. When creating a
     *  HeapBuf dynamically, this needs to be taken into account to determine
     *  the proper buffer size to pass in.
     *
     *  Required parameter.
     *
     *  The default size of the blocks is 0 MAUs.
     */

    Ptr sharedAddr;
    /*!<
     *  Physical address of the shared memory
     *
     *  The creator must supply the shared memory that
     *  this will use for maintain shared state information.
     */

    UInt32 sharedAddrSize;
    /*!< Size of sharedAddr */

     Ptr sharedBuf;
    /*!< *  Address of the shared buffers in caller's address space */

     UInt32 sharedBufSize;
    /*!<
     *  Size of sharedBuf
     *
     *  Can use the #HeapBuf_sharedMemReq call to determine the required size.
     */
} HeapBuf_Params;

/*!
 *  @brief  Stats structure for the getExtendedStats API.
 */
typedef struct HeapBuf_ExtendedStats_tag {
    UInt maxAllocatedBlocks;
    /*!< The maximum number of blocks allocated from this heap at any single
         point in time during the lifetime of this Heap instance. */
    UInt numAllocatedBlocks;
    /*!< The total number of blocks currently allocated in this Heap instance.*/
} HeapBuf_ExtendedStats;


/*!
 *  @brief  Object for the HeapBuf Handle
 */
#define     HeapBuf_Object              Heap_Object

/*!
 *  @brief  Handle for the HeapBuf
 */
#define     HeapBuf_Handle              Heap_Handle


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get default configuration for the HeapBuf module. */
Void HeapBuf_getConfig (HeapBuf_Config * cfgParams);

/* Function to setup the HeapBuf module. */
Int HeapBuf_setup (const HeapBuf_Config * config);

/* Function to destroy the HeapBuf module. */
Int HeapBuf_destroy (void);

/* Initialize this config-params structure with supplier-specified defaults
 * before instance creation.
 */
Void HeapBuf_Params_init (Heap_Handle      handle,
                          HeapBuf_Params * params);

/* Creates a new instance of HeapBuf module. */
Heap_Handle HeapBuf_create (const HeapBuf_Params * params);

/* Deletes a instance of HeapBuf module. */
Int HeapBuf_delete (Heap_Handle * handlePtr);

/* Opens a created instance of HeapBuf module. */
Int HeapBuf_open (Heap_Handle *    handlePtr,
                  HeapBuf_Params * params);

/* Closes previously opened/created instance of HeapBuf module. */
Int HeapBuf_close (Heap_Handle * handle);

/* Returns the amount of shared memory required for creation of each instance.*/
Int HeapBuf_sharedMemReq (const HeapBuf_Params * params, UInt32 *bufSize);

/* Get extended statistics */
Int HeapBuf_getExtendedStats (Heap_Handle             handle,
                              HeapBuf_ExtendedStats * stats);

/* =============================================================================
 *  Functions called through function table interface.
 * =============================================================================
 */
/* Allocate a block. */
Void * HeapBuf_alloc (Heap_Handle handle,
                      UInt32      size,
                      UInt32      align);

/* Frees the block to this HeapBuf. */
Int HeapBuf_free (Heap_Handle handle,
                  Ptr         block,
                  UInt32      size);

/* Get memory statistics */
Int HeapBuf_getStats (Heap_Handle    handle,
                      Memory_Stats * stats);

/* Indicate whether the heap may block during an alloc or free call */
Bool HeapBuf_isBlocking (Heap_Handle handle);

/* Returns the HeapBuf kernel object pointer. */
Void * HeapBuf_getKnlHandle (Heap_Handle handle);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* HEAPBUF_H_0x4CD5 */
