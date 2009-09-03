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
 *  @file       _Notify.c
 *
 *  @brief      User side Notify Manager internal header file
 *
 */


#if !defined (_NOTIFY_H)
#define _NOTIFY_H
/* Notify Headers */
#include <Notify.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/* Functin to create the event receiver thread. */
Int32 _Notify_init (Void) ;
/* Functin to delete the event receiver thread. */
Void _Notify_exit (Void) ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* !defined (_NOTIFY_H) */
