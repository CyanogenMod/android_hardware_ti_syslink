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
/*============================================================================
 *  @file   OsalPrint.c
 *
 *  @brief      Linux user Semaphore interface implementations.
 *
 *              This will have the definitions for user side printf.
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/*OSAL and kernel utils */
#include <OsalPrint.h>

/* Linux specifc header files */
#include <stdarg.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @brief      printf abstraction at the kernel level.
 *
 *  @param      string which needs to be printed.
 *  @param      Variable number of parameters for printf call
 */
Void Osal_printf (char* format, ...)
{
    va_list args;
    char    buffer [512 ];

    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end   (args);

    printf (buffer);
}


#if defined (__cplusplus)
}
#endif /* defined (_cplusplus)*/
