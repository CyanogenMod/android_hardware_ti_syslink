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
 *  @file   _safe.h
 *
 *  @desc   Contains safe programming macros
 *
 *  ============================================================================
 */


#if !defined (_SAFE_H)
#define _SAFE_H


/*  ----------------------------------- IPC headers */
#include <ipc.h>

/*  ----------------------------------- OSAL headers */
#include <mem.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @macro  IS_OBJECT_VALID
 *
 *  @desc   Checks validity of object by comparing against it's signature.
 *  ============================================================================
 */
#define IS_OBJECT_VALID(obj, sign)                                  \
    (((obj != NULL) && ((obj)->signature == sign)) ? TRUE : FALSE)


/** ============================================================================
 *  @macro  IS_RANGE_VALID
 *
 *  @desc   Checks if a value lies in given range.
 *  ============================================================================
 */
#define IS_RANGE_VALID(x,min,max) (((x) < (max)) && ((x) >= (min)))

/** ============================================================================
 *  @macro  MIN
 *
 *  @desc   Returns minumum of the two arguments
 *  ============================================================================
 */
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


/** ============================================================================
 *  @macro  FREE_PTR
 *
 *  @desc   Frees memory pointed to by ptr and sets it to NULL. Also returns
 *          status of MEM_Free function call
 *  ============================================================================
 */
#define FREE_PTR(ptr)  MEM_Free ((Pvoid *) ((Pvoid) &ptr), MEM_DEFAULT)


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (_SAFE_H) */
