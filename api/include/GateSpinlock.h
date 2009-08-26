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
 *  @file   GateSpinlock.h
 *
 *  @brief      Gate based on Spinlock.
 *
 *  ============================================================================
 */


#ifndef GATESPINLOCK_H_0x188E
#define GATESPINLOCK_H_0x188E


/* Module headers */
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    GATESPINLOCK_MODULEID
 *  @brief  Unique module ID.
 */
#define GATESPINLOCK_MODULEID      (0x188E)


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    GATESPINLOCK_STATUSCODEBASE
 *  @brief  Error code base for GatePeterson.
 */
#define GATESPINLOCK_STATUSCODEBASE     (GATESPINLOCK_MODULEID << 12u)

/*!
 *  @def    GATESPINLOCK_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define GATESPINLOCK_MAKE_FAILURE(x)    ((Int)  ( 0x80000000                  \
                                              + (GATESPINLOCK_STATUSCODEBASE  \
                                              + (x))))

/*!
 *  @def    GATESPINLOCK_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define GATESPINLOCK_MAKE_SUCCESS(x)    (GATESPINLOCK_STATUSCODEBASE + (x))

/*!
 *  @def    GATESPINLOCK_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define GATESPINLOCK_E_INVALIDARG       GATESPINLOCK_MAKE_FAILURE(1)

/*!
 *  @def    GATESPINLOCK_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define GATESPINLOCK_E_MEMORY           GATESPINLOCK_MAKE_FAILURE(2)

/*!
 *  @def    GATESPINLOCK_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define GATESPINLOCK_E_BUSY             GATESPINLOCK_MAKE_FAILURE(3)

/*!
 *  @def    GATESPINLOCK_E_FAIL
 *  @brief  Generic failure.
 */
#define GATESPINLOCK_E_FAIL             GATESPINLOCK_MAKE_FAILURE(4)

/*!
 *  @def    GATESPINLOCK_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define GATESPINLOCK_E_NOTFOUND         GATESPINLOCK_MAKE_FAILURE(5)

/*!
 *  @def    GATESPINLOCK_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define GATESPINLOCK_E_INVALIDSTATE     GATESPINLOCK_MAKE_FAILURE(6)

/*!
 *  @def    GATESPINLOCK_E_INUSE
 *  @brief  Indicates that the instance is in use.
 */
#define GATESPINLOCK_E_INUSE            GATESPINLOCK_MAKE_FAILURE(7)

/*!
 *  @def    GATESPINLOCK_E_HANDLE
 *  @brief  An invalid handle was provided.
 */
#define GATESPINLOCK_E_HANDLE           GATESPINLOCK_MAKE_FAILURE(8)

/*!
 *  @def    GATESPINLOCK_SUCCESS
 *  @brief  Operation successful.
 */
#define GATESPINLOCK_SUCCESS            GATESPINLOCK_MAKE_SUCCESS(0)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*! @brief  Object for Gate Mutex */
typedef Gate_Object GateSpinlock_Object;

/*! @brief  Handle for Gate Mutex */
typedef Gate_Handle GateSpinlock_Handle;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to create a Gate Mutex */
GateSpinlock_Handle GateSpinlock_create (Void);

/* Function to delete a Gate Mutex */
Int GateSpinlock_delete (GateSpinlock_Handle * handle);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* GATESPINLOCK_H_0x188E */
