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
 *  @file   OsalMutex.h
 *
 *  @brief      Kernel mutex interface.
 *
 *              Mutex control provided at the kernel
 *              level with the help of Mutex objects. Interface do not
 *              contain much of the state informations and are independent.
 *
 *  ============================================================================
 */

#ifndef OSALMUTEX_H
#define OSALMUTEX_H


/* Standard headers */


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    OSALMUTEX_MODULEID
 *  @brief  Unique module ID.
 */
#define OSALMUTEX_MODULEID                 (UInt16) 0xE4AF

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    OSALMUTEX_STATUSCODEBASE
 *  @brief  Error code base for OSAL Mutex.
 */
#define OSALMUTEX_STATUSCODEBASE  (OSALMUTEX_MODULEID << 12u)

/*!
 *  @def    OSALMUTEX_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define OSALMUTEX_MAKE_FAILURE(x)    ((Int) (0x80000000  \
                                      + (OSALMUTEX_STATUSCODEBASE + (x))))
/*!
 *  @def    OSALMUTEX_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define OSALMUTEX_MAKE_SUCCESS(x)    (OSALMUTEX_STATUSCODEBASE + (x))

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*
* @def   OSALMUTEX_E_MEMORY
* @brief Indicates Memory alloc/free failure.
*/
#define OSALMUTEX_E_MEMORY             OSALMUTEX_MAKE_FAILURE(1)

/*
* @def   OSALMUTEX_E_INVALIDARG
* @brief Invalid argument provided
*/
#define OSALMUTEX_E_INVALIDARG         OSALMUTEX_MAKE_FAILURE(2)

/*
* @def   OSALMUTEX_E_FAIL
* @brief Generic failure
*/
#define OSALMUTEX_E_FAIL               OSALMUTEX_MAKE_FAILURE(3)

/*
* @def   OSALMUTEX_E_TIMEOUT
* @brief A timeout occurred
*/
#define OSALMUTEX_E_TIMEOUT            OSALMUTEX_MAKE_FAILURE(4)

/*
 *  @def    OSALMUTEX_E_HANDLE
 *  @brief  Invalid handle provided
 */
#define OSALMUTEX_E_HANDLE             OSALMUTEX_MAKE_FAILURE(5)

/*
* @def   OSALMUTEX_SUCCESS
* @brief Operation successfully completed
*/
#define OSALMUTEX_SUCCESS              OSALMUTEX_MAKE_SUCCESS(0)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*
 *  @brief  Declaration for the OsalSpinlock object handle.
 *          Definition of OsalMutex_Object is not exposed.
 */
typedef struct OsalMutex_Object * OsalMutex_Handle;

/*
 *  @brief   Enumerates the types of spinlocks
 */
typedef enum {
    OsalMutex_Type_Interruptible    = 0u,
    /*!< Waits on this mutex are interruptible */
    OsalMutex_Type_Noninterruptible = 1u,
    /*!< Waits on this mutex are non-interruptible */
    OsalMutex_Type_TryLock          = 2u,
    /*!< Blocks until it acquires the mutex */
    OsalMutex_Type_EndValue         = 3u
    /* End delimiter indicating start of invalid values for this enum */
} OsalMutex_Type;


/* =============================================================================
 *  APIs
 * =============================================================================
 */

/* Creates a new instance of Mutex object. */
OsalMutex_Handle OsalMutex_create(OsalMutex_Type type);

/* Deletes an instance of Mutex object. */
Int OsalMutex_delete(OsalMutex_Handle *mutexHandle);

/* Enters the critical section indicated by this Mutex object. Returns key. */
UInt32 OsalMutex_enter(OsalMutex_Handle mutexHandle);

/* Leaves the critical section indicated by this Mutex object.
 * Takes in key received from enter.
 */
Void OsalMutex_leave(OsalMutex_Handle mutexHandle, UInt32 key);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef OSALMUTEX */



