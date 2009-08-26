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
 *  @file   _NotifyDefs.h
 *
 *  @brief      Internal declarations of types for Notify module.
 *
 *  
 *  ============================================================================
 */


#if !defined (_NOTIFY_H_0x5f84)
#define _NOTIFY_H_0x5f84


/* Standard headers */
#include <Std.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  This object is a common object and must be the first field of any
 *          driver. This object is used by Notify module to get the kernel
 *          object handle for the driver.
 */
typedef struct Notify_CommonObject_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side Notify object. */
} Notify_CommonObject;

typedef void (*FnNotifyCbck) (int procId,
                                unsigned long int  eventNo,
                                void *arg,
                                unsigned long int  payload) ;

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* !defined (_NOTIFY_H_0x5f84) */
