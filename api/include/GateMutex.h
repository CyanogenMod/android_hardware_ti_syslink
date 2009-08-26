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
 *  @file   GateMutex.h
 *
 *  @brief      Gate based on Mutex.
 *  ============================================================================
 */


#ifndef GATEMUTEX_H_0x72D0
#define GATEMUTEX_H_0x72D0


/* Module headers */
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @def    GATEMUTEX_MODULEID
 *  @brief  Unique module ID.
 */
#define GATEMUTEX_MODULEID      (0x72D0)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    GATEMUTEX_STATUSCODEBASE
 *  @brief  Error code base for Gate Mutex.
 */
#define GATEMUTEX_STATUSCODEBASE  (GATEMUTEX_MODULEID << 12u)

/*!
 *  @def    GATEMUTEX_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define GATEMUTEX_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                         + (GATEMUTEX_STATUSCODEBASE  \
                                         + (x))))

/*!
 *  @def    GATEMUTEX_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define GATEMUTEX_MAKE_SUCCESS(x)    (GATEMUTEX_STATUSCODEBASE + (x))

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*
 *  @def    GATEMUTEX_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define GATEMUTEX_E_INVALIDARG       GATEMUTEX_MAKE_FAILURE(1)

/*
 *  @def    GATEMUTEX_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define GATEMUTEX_E_MEMORY           GATEMUTEX_MAKE_FAILURE(2)

/*
 *  @def    GATEMUTEX_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define GATEMUTEX_E_BUSY             GATEMUTEX_MAKE_FAILURE(3)

/*
 *  @def    GATEMUTEX_E_FAIL
 *  @brief  Generic failure.
 */
#define GATEMUTEX_E_FAIL             GATEMUTEX_MAKE_FAILURE(4)

/*
 *  @def    GATEMUTEX_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define GATEMUTEX_E_NOTFOUND         GATEMUTEX_MAKE_FAILURE(5)

/*
 *  @def    GATEMUTEX_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define GATEMUTEX_E_INVALIDSTATE     GATEMUTEX_MAKE_FAILURE(6)

/*
 *  @def    GATEMUTEX_E_INUSE
 *  @brief  Indicates that the instance is in use.
 */
#define GATEMUTEX_E_INUSE            GATEMUTEX_MAKE_FAILURE(7)

/*
 *  @def    GATEMUTEX_SUCCESS
 *  @brief  Operation successful.
 */
#define GATEMUTEX_SUCCESS            GATEMUTEX_MAKE_SUCCESS(0)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/* @brief  Object for Gate Mutex */
typedef Gate_Object GateMutex_Object;

/* @brief  Handle for Gate Mutex */
typedef Gate_Handle GateMutex_Handle;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to create a Gate Mutex */
GateMutex_Handle GateMutex_create (Void);

/* Function to delete a Gate Mutex */
Int GateMutex_delete (GateMutex_Handle * gmHandle);

/* Function to enter a Gate Mutex */
UInt32 GateMutex_enter (GateMutex_Handle gmHandle);

/* Function to leave a Gate Mutex */
Void GateMutex_leave (GateMutex_Handle gmHandle, UInt32 key);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* GATEMUTEX_H */
