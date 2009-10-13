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

/*!
 *  @file       _ProcMgrDefs.h
 *
 *  @brief      Internal type defines for ProcMgr.
 *
 *              This file is included by specific instance implementations for
 *              loaders, Processor module or PwrMgr module.
 *
 */


#ifndef _ProcMgrDefs_H_0xf2ba
#define _ProcMgrDefs_H_0xf2ba


/* Standard headers */
#include <Std.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Common object of for ProcMgr and its related modules. This must be
 *          the first field in the instance objects of loaders, Processor
 *          instances and PwrMgr instances.
 */
typedef struct ProcMgr_CommonObject_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side object. */
} ProcMgr_CommonObject;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _ProcMgrDefs_H_0xf2ba */
