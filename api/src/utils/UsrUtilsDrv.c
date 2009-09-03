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
 *  @file   UsrUtilsDrv.c
 *
 *  @brief      Initializes and finalizes user side utils
 *              Mainly created for future use. All setup/destroy APIs on user
 *              side will be call from this module. This allows syslink to work
 *              independent od system manager.
 *  ============================================================================
 */


/*  Defined to include MACROS EXPORT_SYMBOL. This must be done before including
 *  module.h
 */
#if !defined (EXPORT_SYMTAB)
#define EXPORT_SYMTAB
#endif


/* Standard headers */
#include <Std.h>
#include <Trace.h>

/* User side utils headers */
#include <MemoryOS.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to initialize user side utils
 */
Void UsrUtilsDrv_setup (Void)
{
    GT_0trace (curTrace, GT_ENTER, "UsrUtilsDrv_setup");

    /* Initialize the MemoryOS module */
    MemoryOS_setup ();

    GT_0trace (curTrace, GT_LEAVE, "UsrUtilsDrv_setup");
}


/*!
 *  @brief  Function to finalize user side utils
 */
Void UsrUtilsDrv_destroy (Void)
{
    GT_0trace (curTrace, GT_ENTER, "UsrUtilsDrv_destroy");

    /* Finalize the MemoryOS module */
    MemoryOS_destroy ();

    Osal_printf ("Leaving UsrUtilsDrv_destroy \n");
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
