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
 *  @file   _intobject.h
 *
 *
 *  @desc   Declares an object that encapsulates the interrupt information
 *          reqiured by Linux.
 *
 *  ============================================================================
 */


#if !defined (_INTOBJECT_H)
#define _INTOBJECT_H


/*  ----------------------------------- DSP/BIOS Link                   */
#include <gpptypes.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @name   InterruptObject
 *
 *  @desc   Object encapsulating the OS dependent interrupt information.
 *
 *  @field  intId
 *              Interrupt identifier
 *  ============================================================================
 */
typedef struct InterruptObject_tag {
    Int32  intId ;
} InterruptObject ;


#if defined (__cplusplus)
}
#endif


#endif /* !defined (_INTOBJECT_H) */
