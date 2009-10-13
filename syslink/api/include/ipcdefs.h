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
 *  @file   ipcdefs.h
 *
 *  @desc   Defines data types and structures used by IPC modules.
 *  ============================================================================
 */


#if !defined (IPCDEFS_H)
#define IPCDEFS_H


/*  ----------------------------------- IPC headers */
#include <gpptypes.h>
#include <ipctypes.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @name   Endianism
 *
 *  @desc   Enumeration of data endianism.
 *
 *  @field  Endianism_Default
 *              Default endianism - no conversion required.
 *  @field  Endianism_Big
 *              Big endian.
 *  @field  Endianism_Little
 *              Little endian.
 *  ============================================================================
 */
typedef enum {
    Endianism_Default = 1u,
    Endianism_Big     = 2u,
    Endianism_Little  = 3u
} Endianism ;


#if defined (__cplusplus)
}
#endif


#endif /* !defined (IPCDEFS_H) */
