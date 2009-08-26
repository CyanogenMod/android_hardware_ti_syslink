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
 *  @file   notify_driver.h
 *
 *  @desc   Defines data types and structures used by Notify driver writers.
 *  ============================================================================
 */


#if !defined (NOTIFYDRIVER_H)
#define NOTIFYDRIVER_H


/*  ----------------------------------- IPC */
#include <ipctypes.h>

/*  ----------------------------------- Notify */
#include <notifyerr.h>

/*  ----------------------------------- Notify driver */
#include <notify_driverdefs.h>


#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @func   Notify_registerDriver
 *
 *  @desc   This function registers a Notify driver with the Notify module.
 *          Each Notify driver must register itself with the Notify module. Once
 *          the registration is done, the user can start using the Notify APIs
 *          to interact with the Notify driver for sending and receiving
 *          notifications.
 *
 *  @arg    driverName
 *              Name of the Notify driver
 *  @arg    fnTable
 *              Pointer to the function table for this Notify driver
 *  @arg    drvAttrs
 *              Attributes of the Notify driver relevant to the generic Notify
 *              module
 *  @arg    driverHandle
 *              Location to receive the pointer to the Notify driver handle
 *
 *  @ret    NOTIFY_SOK
 *              Operation successfully completed
 *          NOTIFY_EINIT
 *              The Notify module or driver was not initialized
 *          NOTIFY_ERESOURCE
 *              The maximum number of drivers have already been registered with
 *              the Notify module
 *          NOTIFY_EALREADYEXISTS
 *              The specified Notify driver is already registered.
 *          NOTIFY_EMEMORY
 *              Operation failed due to a memory error
 *          NOTIFY_EINVALIDARG
 *              Invalid argument
 *          NOTIFY_EPOINTER
 *              An invalid pointer was specified
 *          NOTIFY_EFAIL
 *              General failure
 *
 *  @enter  The Notify module must be initialized before calling this function.
 *          driverName must be a valid string.
 *          fnTable must be valid.
 *          driverAttrs must be valid.
 *          driverHandle must be a valid pointer.
 *
 *  @leave  On success, the Notify driver must be registered with the Notify
 *          module
 *
 *  @see    Notify_Interface, Notify_DriverAttrs, Notify_unregisterDriver ()
 *  ============================================================================
 */
EXPORT_API
Notify_Status
Notify_registerDriver (IN  Char8 *               driverName,
                       IN  Notify_Interface *    fnTable,
                       IN  Notify_DriverAttrs *  drvAttrs,
                       OUT Notify_DriverHandle * driverHandle) ;


/** ============================================================================
 *  @func   Notify_unregisterDriver
 *
 *  @desc   This function un-registers a Notify driver with the Notify module.
 *          When the Notify driver is no longer required, it can un-register
 *          itself with the Notify module. This facility is also useful if a
 *          different type of Notify driver needs to be plugged in for the same
 *          physical interrupt to meet different needs.
 *
 *  @arg    drvHandle
 *              Handle to the Notify driver object
 *
 *  @ret    NOTIFY_SOK
 *              Operation successfully completed
 *          NOTIFY_EINIT
 *              The Notify module or driver was not initialized
 *          NOTIFY_ENOTFOUND
 *              The specified driver is not registered with the Notify module
 *          NOTIFY_EHANDLE
 *              Invalid Notify handle specified
 *          NOTIFY_EMEMORY
 *              Operation failed due to a memory error
 *          NOTIFY_EINVALIDARG
 *              Invalid argument
 *          NOTIFY_EFAIL
 *              General failure
 *
 *  @enter  The Notify module must be initialized before calling this function.
 *          handle must be a valid Notify driver handle.
 *
 *  @leave  On success, the Notify driver must be unregistered with the Notify
 *          module
 *
 *  @see    Notify_DriverHandle, Notify_registerDriver ()
 *  ============================================================================
 */
EXPORT_API
Notify_Status
Notify_unregisterDriver (IN  Notify_DriverHandle drvHandle) ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFYDRIVER_H) */
