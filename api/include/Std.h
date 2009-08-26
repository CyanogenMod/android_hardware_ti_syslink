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
 *  @file   Std.h
 *
 *  @brief      This will have definitions of standard data types for
 *              platform abstraction.
 *
 *  ============================================================================
 */

#if !defined(STD_H)
#define STD_H

#ifdef SYSLINK_BUILDOS_LINUX
#include <std_linux.h>
#endif
#include <unistd.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef char              Char;
typedef unsigned char     UChar;
typedef short             Short;
typedef unsigned short    UShort;
typedef int               Int;
typedef unsigned int      UInt;
typedef long              Long;
typedef unsigned long     ULong;
typedef float             Float;
typedef double            Double;
typedef long double       LDouble;
typedef void              Void;


typedef unsigned short    Bool;
typedef void            * Ptr;       /* data pointer */
typedef void            * Handle;    /* data pointer */
typedef char            * String;    /* null terminated string */


typedef int            *  IArg;
typedef unsigned int   *  UArg;
typedef char              Int8;
typedef short             Int16;
typedef int               Int32;

typedef unsigned char     UInt8;
typedef unsigned short    UInt16;
typedef unsigned int      UInt32;
typedef UInt32            Bits32;

/* taken from bridge */
typedef void           *PVOID;		/* p    */
typedef PVOID           HANDLE;		/* h    */


#define TRUE              1
#define FALSE             0
#define FAIL   -1
//#define NULL		  '\0'

#if defined (__cplusplus)
}
#endif

#endif

