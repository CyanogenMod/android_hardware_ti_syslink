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
 *  @file   OsalPrint.h
 *
 *  @brief      Kernel utils Print interface definitions.
 *
 *              This will have the definitions for kernel side printf
 *              statements and also details of variable printfs
 *              supported in existing implementation.
 *
 *  ============================================================================
 */


#ifndef OSALPRINT_H_0xC431
#define OSALPRINT_H_0xC431


/* Standard headers */
#include <Std.h>
#include <stdio.h>

/* OSAL and utils */


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    OSALPRINT_MODULEID
 *  @brief  Module ID for OsalPrint OSAL module.
 */
#define OSALPRINT_MODULEID                 (UInt16) 0xC431


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/*  printf abstraction at the kernel level. */
Void Osal_printf(char* format, ...);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef OSALPRINT_H_0xC431 */
