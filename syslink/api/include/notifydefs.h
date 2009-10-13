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
 *  @file   notifydefs.h
 *
 *  @desc   Defines data types and structures used by Notify module
 *
 *  ============================================================================
 */


#if !defined (NOTIFYDEFS_H)
#define NOTIFYDEFS_H


/*  ----------------------------------- IPC */
#include <ipctypes.h>

/*  ----------------------------------- Notify */
#include <notifyerr.h>


#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @const  NOTIFY_MAX_DRIVERS
 *
 *  @desc   Maximum number of Notify drivers supported.
 *          NOTE: This can be modified by the user if data section size needs to
 *                be reduced, or if there is a need for more than the defined
 *                max drivers.
 *  ============================================================================
 */
#define NOTIFY_MAX_DRIVERS   16

/** ============================================================================
 *  @const  NOTIFY_MAX_NAMELEN
 *
 *  @desc   Maximum length of the name of Notify drivers, inclusive of NULL
 *          string terminator.
 *  ============================================================================
 */
#define NOTIFY_MAX_NAMELEN   32

/** ============================================================================
 *  @name   Notify_Handle
 *
 *  @desc   This typedef defines the type for the handle to the Notify driver.
 *  ============================================================================
 */
typedef Void * Notify_Handle ;


/** ============================================================================
 *  @name   FnNotifyCbck
 *
 *  @desc   Signature of the callback function to be registered with the NOTIFY
 *          component.
 *
 *  @arg    procId
 *              Processor ID from which the event is being received
 *  @arg    eventNo
 *              Event number for which the callback is being received
 *  @arg    arg
 *              Fixed argument registered with the IPS component along with
 *              the callback function.
 *  @arg    payload
 *              Run-time information provided to the upper layer by the Notify
 *              component. Depending on the Notify driver implementation, this
 *              may or may not pass on a user-specific value to the registered
 *              application
 *
 *  @ret    None.
 *
 *  @enter  None.
 *
 *  @leave  None.
 *
 *  @see    None.
 *  ============================================================================
 */
typedef Void (*FnNotifyCbck) (IN     Processor_Id procId,
                              IN     Uint32       eventNo,
                              IN OPT Void *       arg,
                              IN OPT Uint32       payload) ;


/** ============================================================================
 *  @name   Notify_Attrs
 *
 *  @desc   This structure defines attributes for initialization of the Notify
 *          module.
 *
 *  @field  maxDrivers
 *              Maximum number of drivers that can be registered with the Notify
 *              module.
 *
 *  @see    Notify_init ()
 *  ============================================================================
 */
typedef struct Notify_Attrs_tag {
    Uint32   maxDrivers ;
} Notify_Attrs ;

/** ============================================================================
 *  @name   Notify_Config
 *
 *  @desc   This structure defines the configuration structure for
 *          initialization of the Notify driver.
 *
 *  @field  driverAttrs
 *              Notify driver-specific attributes
 *
 *  @see    Notify_driverInit ()
 *  ============================================================================
 */
typedef struct Notify_Config_tag {
    Void *   driverAttrs ;
} Notify_Config ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFYDEFS_H) */
