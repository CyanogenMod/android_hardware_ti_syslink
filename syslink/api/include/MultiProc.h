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
 *  @file   MultiProc.h
 *
 *  @brief      Header file for MultiProc module on HLOS side
 *  ============================================================================
 */


#ifndef MULTIPROC_H_0XB522
#define MULTIPROC_H_0XB522


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    MULTIPROC_MODULEID
 *  @brief  Unique module ID.
 */
#define MULTIPROC_MODULEID      (UInt16) 0xB522


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    MULTIPROC_STATUSCODEBASE
 *  @brief  Error code base for MultiProc module.
 */
#define MULTIPROC_STATUSCODEBASE  (MULTIPROC_MODULEID << 12u)

/*!
 *  @def    MULTIPROC_MAKE_FAILURE
 *  @brief  Macro to make failure code.
 */
#define MULTIPROC_MAKE_FAILURE(x) ((Int) (  0x80000000                         \
                                          + MULTIPROC_STATUSCODEBASE           \
                                          + (x)))

/*!
 *  @def    MULTIPROC_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define MULTIPROC_MAKE_SUCCESS(x) (MULTIPROC_STATUSCODEBASE + (x))

/*!
 *  @def    MULTIPROC_E_INVALIDID
 *  @brief  Invalid ID specified
 */
#define MULTIPROC_E_INVALIDID   MULTIPROC_MAKE_FAILURE(1)

/*!
 *  @def    MULTIPROC_EINVALIDNAME
 *  @brief  Invalid name specified
 */
#define MULTIPROC_E_INVALIDNAME MULTIPROC_MAKE_FAILURE(2)
/*!
 *  @def    MULTIPROC_E_CONFIG
 *  @brief  Invalid config parameter
 */
#define MULTIPROC_E_CONFIG      MULTIPROC_MAKE_FAILURE(3)

/*!
 *  @def    MULTIPROC_E_OSFAILURE
 *  @brief  Failure in MultiProc driver operation
 */
#define MULTIPROC_E_OSFAILURE   MULTIPROC_MAKE_FAILURE(4)

/*!
 *  @def    MULTIPROC_E_FAIL
 *  @brief  Generic failure
 */
#define MULTIPROC_E_FAIL       MULTIPROC_MAKE_FAILURE(5)

/*!
 *  @def    MULTIPROC_E_MEMORY
 *  @brief  Failure in memory calls
 */
#define MULTIPROC_E_MEMORY     MULTIPROC_MAKE_FAILURE(6)

/*!
 *  @def    MULTIPROC_E_INVALIDSTATE
 *  @brief  Module in invalid state
 */
#define MULTIPROC_E_INVALIDSTATE MULTIPROC_MAKE_FAILURE(7)

/*!
 *  @def    MULTIPROC_SUCCESS
 *  @brief  Operation successfully completed.
 */
#define MULTIPROC_SUCCESS       MULTIPROC_MAKE_SUCCESS(0)

/*!
 *  @def    MULTIPROC_S_ALREADYSETUP
 *  @brief  Module already initialized.
 */
#define MULTIPROC_S_ALREADYSETUP MULTIPROC_MAKE_SUCCESS(1)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Defines an invalid processor id.
 */
#define MULTIPROC_INVALIDID (UInt16) 0xFFFF

/*!
 *  @brief  Max name length for a processor name.
 */
#define MULTIPROC_MAXNAMELENGTH 32

/*!
 *  @brief  Max number of processors supported.
 */
#define MULTIPROC_MAXPROCESSORS 4


/*!
 *  @brief  Configuration structure for MultiProc module
 */
typedef struct MultiProc_Config_tag {
    Int32  maxProcessors;
    /*!< Max number of procs for particular system */
    Char   nameList [MULTIPROC_MAXPROCESSORS][MULTIPROC_MAXNAMELENGTH];
    /*!< Name List for processors in the system */
    UInt16 id;
    /*!< Local Proc ID. This needs to be set before calling any other APIs */
} MultiProc_Config;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get the default configuration for the MultiProc module. */
Void MultiProc_getConfig (MultiProc_Config * cfg);

/* Function to setup the MultiProc Module */
Int32 MultiProc_setup (MultiProc_Config * cfg);

/* Function to destroy the MultiProc module */
Int32 MultiProc_destroy (Void);

/* Function to set local processor Id */
Int32 MultiProc_setLocalId (UInt16 id);

/* Function to get processor Id given proc name*/
UInt16 MultiProc_getId (String name);

/* Function to get processor name give proc Id */
String MultiProc_getName (UInt16 id);

/* Function to get maximum processors */
UInt16 MultiProc_getMaxProcessors (Void);


/* =============================================================================
 *  Compatibility layer for SYSBIOS
 * =============================================================================
 */
#define MultiProc_maxProcessors      MULTIPROC_MAXPROCESSORS
#define MultiProc_INVALIDID          MULTIPROC_INVALIDID
#define MultiProc_MODULEID           MULTIPROC_MODULEID
#define MultiProc_STATUSCODEBASE     MULTIPROC_STATUSCODEBASE
#define MultiProc_MAKE_FAILURE       MULTIPROC_MAKE_FAILURE
#define MultiProc_MAKE_SUCCESS       MULTIPROC_MAKE_SUCCESS
#define MultiProc_E_INVALIDID        MULTIPROC_E_INVALIDID
#define MultiProc_E_INVALIDNAME      MULTIPROC_E_INVALIDNAME
#define MultiProc_SUCCESS            MULTIPROC_SUCCESS


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* if !defined(MultiProc_H_0xb522) */
