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
 *  @file   OsalDrv.h
 *
 *  @brief      Declarations of OS-specific functionality for Osal
 *
 *              This file contains declarations of OS-specific functions for
 *              Osal.
 *
 *  ============================================================================
 */


#ifndef OsalDrv_H_0xf2ba
#define OsalDrv_H_0xf2ba


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @def    OSALDRV_MODULEID
 *  @brief  Module ID for Memory OSAL module.
 */
#define OSALDRV_MODULEID                 (UInt16) 0x97D3

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 * @def   OSALDRV_STATUSCODEBASE
 * @brief Stauts code base for MEMORY module.
 */
#define OSALDRV_STATUSCODEBASE            (OSALDRV_MODULEID << 12u)

/*!
 * @def   OSALDRV_MAKE_FAILURE
 * @brief Convert to failure code.
 */
#define OSALDRV_MAKE_FAILURE(x)          ((Int) (0x80000000  \
                                           + (OSALDRV_STATUSCODEBASE + (x))))
/*!
 * @def   OSALDRV_MAKE_SUCCESS
 * @brief Convert to success code.
 */
#define OSALDRV_MAKE_SUCCESS(x)            (OSALDRV_STATUSCODEBASE + (x))

/*!
 * @def   OSALDRV_E_MAP
 * @brief Failed to map to host address space.
 */
#define OSALDRV_E_MAP                      OSALDRV_MAKE_FAILURE(0)

/*!
 * @def   OSALDRV_E_UNMAP
 * @brief Failed to unmap from host address space.
 */
#define OSALDRV_E_UNMAP                    OSALDRV_MAKE_FAILURE(1)

/*!
 * @def   OSALDRV_E_OSFAILURE
 * @brief Failure in OS calls.
 */
#define OSALDRV_E_OSFAILURE                OSALDRV_MAKE_FAILURE(2)

/*!
 * @def   OSALDRV_SUCCESS
 * @brief Operation successfully completed
 */
#define OSALDRV_SUCCESS                    OSALDRV_MAKE_SUCCESS(0)


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to open the ProcMgr driver. */
Int OsalDrv_open (Void);

/* Function to close the ProcMgr driver. */
Int OsalDrv_close (Void);

/* Function to map a memory region specific to the driver. */
UInt32 OsalDrv_map (UInt32 addr, UInt32 size);

/* Function to unmap a memory region specific to the driver. */
Void OsalDrv_unmap (UInt32 addr, UInt32 size);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* OsalDrv_H_0xf2ba */
