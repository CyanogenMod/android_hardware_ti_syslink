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
 *  @file   OsalSemaphore.h
 *
 *  @brief      Kernel utils Semaphore interface definitions.
 *
 */


#ifndef OSALSEMAPHORE_H
#define OSALSEMAPHORE_H


/* Standard headers */
#include <Std.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @def    OSALSEMAPHORE_MODULEID
 *  @brief  Unique module ID.
 */
#define OSALSEMAPHORE_MODULEID               (0xcec2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    OSALSEMAPHORE_STATUSCODEBASE
 *  @brief  Error code base for OSAL Semaphore.
 */
#define OSALSEMAPHORE_STATUSCODEBASE  (OSALSEMAPHORE_MODULEID << 12u)

/*!
 *  @def    OSALSEMAPHORE_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define OSALSEMAPHORE_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                         + (OSALSEMAPHORE_STATUSCODEBASE  \
                                         + (x))))

/*!
 *  @def    OSALSEMAPHORE_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define OSALSEMAPHORE_MAKE_SUCCESS(x)    (OSALSEMAPHORE_STATUSCODEBASE + (x))

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
* @def   OSALSEMAPHORE_E_MEMORY
* @brief Indicates OsalSemaphore alloc/free failure.
*/
#define OSALSEMAPHORE_E_MEMORY             OSALSEMAPHORE_MAKE_FAILURE(1)

/*!
* @def   OSALSEMAPHORE_E_INVALIDARG
* @brief Invalid argument provided
*/
#define OSALSEMAPHORE_E_INVALIDARG         OSALSEMAPHORE_MAKE_FAILURE(2)

/*!
* @def   OSALSEMAPHORE_E_FAIL
* @brief Generic failure
*/
#define OSALSEMAPHORE_E_FAIL               OSALSEMAPHORE_MAKE_FAILURE(3)

/*!
* @def   OSALSEMAPHORE_E_TIMEOUT
* @brief A timeout occurred
*/
#define OSALSEMAPHORE_E_TIMEOUT            OSALSEMAPHORE_MAKE_FAILURE(4)

/*!
 *  @def    OSALSEMAPHORE_E_HANDLE
 *  @brief  Invalid handle provided
 */
#define OSALSEMAPHORE_E_HANDLE             OSALSEMAPHORE_MAKE_FAILURE(5)

/*!
 *  @def    OSALSEMAPHORE_E_WAITNONE
 *  @brief  WAIT_NONE timeout value was provided, but semaphore was not
 *          available.
 */
#define OSALSEMAPHORE_E_WAITNONE           OSALSEMAPHORE_MAKE_FAILURE(6)

/*!
 *  @def    OSALSEMAPHORE_E_RESOURCE
 *  @brief  Resource error.
 */
#define OSALSEMAPHORE_E_RESOURCE           OSALSEMAPHORE_MAKE_FAILURE(7)

/*!
* @def   OSALSEMAPHORE_SUCCESS
* @brief Operation successfully completed
*/
#define OSALSEMAPHORE_SUCCESS              OSALSEMAPHORE_MAKE_SUCCESS(0)

/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @def    OSALSEMAPHORE_WAIT_FOREVER
 *  @brief  Indicates forever wait for APIs that can wait.
 */
#define OSALSEMAPHORE_WAIT_FOREVER           (~((UInt32) 0))

/*!
 *  @def    OSALSEMAPHORE_WAIT_NONE
 *  @brief  Indicates zero wait for APIs that can wait.
 */
#define OSALSEMAPHORE_WAIT_NONE               ((UInt32) 0u)

/*!
 *  @def    OSALSEMAPHORE_TYPE_VALUE
 *  @brief  Returns the value of semaphore type (binary/counting)
 */
#define OSALSEMAPHORE_TYPE_VALUE(type)        (type & 0x0000FFFF)

/*!
 *  @def    OSALSEMAPHORE_INTTYPE_VALUE
 *  @brief  Returns the value of semaphore interruptability type
 */
#define OSALSEMAPHORE_INTTYPE_VALUE(type)     (type & 0xFFFF0000)

/*!
 *  @brief  Declaration for the OsalSemaphore object handle.
 *          Definition of OsalSemaphore_Object is not exposed.
 */
typedef struct OsalSemaphore_Object * OsalSemaphore_Handle;

/*!
 *  @brief   Enumerates the types of semaphores
 */
typedef enum {
    OsalSemaphore_Type_Binary          = 0x00000000,
    /*!< Binary semaphore */
    OsalSemaphore_Type_Counting        = 0x00000001,
    /*!< Counting semaphore */
    OsalSemaphore_Type_EndValue        = 0x00000002
    /*!< End delimiter indicating start of invalid values for this enum */
} OsalSemaphore_Type;

/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Creates the semaphore object. */
OsalSemaphore_Handle OsalSemaphore_create (UInt32 semType, UInt32 semValue);

/* Deletes the semaphore object */
Int OsalSemaphore_delete (OsalSemaphore_Handle *semHandle);

/* Wait on the said Semaphore in the kernel thread context */
Int OsalSemaphore_pend (OsalSemaphore_Handle semHandle, UInt32 timeout);

/* Signal the semaphore and make it available for other threads. */
Int OsalSemaphore_post (OsalSemaphore_Handle semHandle);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef OSALSEMAPHORE_H */
