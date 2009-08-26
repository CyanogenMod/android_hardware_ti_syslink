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
 *  @file   Gate.h
 *
 *  @brief      Gate wrapper defines.
 *
 *  ============================================================================
 */


#ifndef GATE_H_0xAF6F
#define GATE_H_0xAF6F


#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @def    GATE_MODULEID
 *  @brief  Unique module ID.
 */
#define GATE_MODULEID      (0xAF6F)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    GATE_STATUSCODEBASE
 *  @brief  Error code base for GatePeterson.
 */
#define GATE_STATUSCODEBASE     (GATE_MODULEID << 12u)

/*!
 *  @def    GATE_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define GATE_MAKE_FAILURE(x)          ((Int)  (  0x80000000                    \
                                               + (GATE_STATUSCODEBASE          \
                                               + (x))))

/*!
 *  @def    GATE_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define GATE_MAKE_SUCCESS(x)    (GATE_STATUSCODEBASE + (x))

/*!
 *  @def    GATE_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define GATE_E_INVALIDARG       GATE_MAKE_FAILURE(1)

/*!
 *  @def    GATE_SUCCESS
 *  @brief  Operation successful.
 */
#define GATE_SUCCESS            GATE_MAKE_SUCCESS(0)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*! @brief Type for function pointer to enter Gate function */
typedef UInt32 (*Lock_enter) (Void * handle);

/*! @brief Type for function pointer to leave Gate function */
typedef void   (*Lock_leave) (Void * handle, UInt32 key);

/*! @brief Type for function pointer to get handle to kernel object */
typedef void* (*Lock_getKnlHandle) (Void * handle);

/*! @brief Structure defining Gate Objects */
typedef struct Gate_Object_tag {
    Lock_enter  enter; /*!< Pointer to enter function */
    Lock_leave  leave; /*!< Pointer to leave function */
    Lock_getKnlHandle getKnlHandle; /*!< Returns pointer to the internal obect.
                                     * Kernl object incase of GatePeterson
                                     */
    Ptr         obj;   /*!< Pointer to the internal object */
} Gate_Object;

/*! @brief Structure defining Gate Objects */
typedef Gate_Object * Gate_Handle;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to enter a Gate */
UInt32 Gate_enter (Gate_Handle handle);

/* Function to leave a Gate */
Void Gate_leave (Gate_Handle handle, UInt32 key);

/* Function to return kernel object handle */
Void* Gate_getKnlHandle (Gate_Handle handle);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* GATE_H_0xAF6F */
