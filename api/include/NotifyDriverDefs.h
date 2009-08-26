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
 *  @file       NotifyDriverDefs.h
 *
 *  @brief      Interal types and definitions for the Notify drivers.
 *
 *
 */


#if !defined (NOTIFYDRIVERDEFS_H_0x5f84)
#define NOTIFYDRIVERDEFS_H_0x5f84


/* Module headers */
#include <MultiProc.h>
#include <Notify.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Enumerations to indicate types of Driver initialization status
 */
typedef enum {
    Notify_DriverInitStatus_NotDone     = 0u,
    /*!< Driver initialization is not done. */
    Notify_DriverInitStatus_Done        = 1u,
    /*!< Driver initialization is complete. */
    Notify_DriverInitStatus_InProgress  = 2u,
    /*!< Driver initialization is in progress. */
    Notify_DriverInitStatus_EndValue    = 3u
    /*!< End delimiter indicating start of invalid values for this enum */
} Notify_DriverInitStatus;


/*!
 *  @brief This structure defines information for all processors supported by
 *         the Notify driver.
 *         An instance of this object is provided for each processor handled by
 *         the Notify driver, when registering itself with the Notify module.
 *
 */
typedef struct NotifyDriver_ProcInfo_tag {
    UInt32       maxEvents;
    /*!< Maximum number of events supported by driver */
    UInt32       reservedEvents;
    /*!< Number of reserved events */
    Bool         eventPriority;
    /*!< Is event prioritization supported by the driver for this processor? */
    UInt32       payloadSize;
    /*!< Payload size supported by the driver for this processor. */
    UInt16       procId;
    /*!< Proccessor Id */
} NotifyDriver_ProcInfo;

/*!
 *  @brief  This structure defines the structure for specifying Notify driver
 *          attributes to the Notify module.
 *          This structure provides information about the Notify driver to the
 *          Notify module. The information is used by the Notify module mainly
 *          for parameter validation. It may also be used by the Notify module
 *          to take appropriate action if required, based on the characteristics
 *          of the Notify driver.
 */
typedef struct NotifyDriver_Attrs_tag {
    UInt32                numProc;
    /*!< number of procs supported by driver */
    NotifyDriver_ProcInfo procInfo [MULTIPROC_MAXPROCESSORS];
    /*!< processor info of supported processors */
} NotifyDriver_Attrs;


/* =============================================================================
 *  Function pointer types
 * =============================================================================
 */
/*!
 *  @brief  This type defines the function to register a callback for an event
 *          with the Notify driver.
 *          This function gets called internally from the Notify_registerEvent
 *          API. The Notify_registerEvent () function passes on the
 *          request into the Notify driver identified by the Notify Handle.
 *
 */
typedef Int32 (*NotifyDriver_RegisterEvent) (NotifyDriver_Handle handle,
                                             UInt16              procId,
                                             UInt32              eventNo,
                                             Notify_CallbackFxn  cbckFxn,
                                             Void *              cbckArg);
/*!
 *  @brief  This type defines the function to unregister a callback for an event
 *          with the Notify driver.
 *          This function gets called internally from the Notify_unregisterEvent
 *          API. The Notify_unregisterEvent () function passes on the
 *          request into the Notify driver identified by the Notify Handle.
 *
 */
typedef Int32 (*NotifyDriver_UnregisterEvent) (NotifyDriver_Handle handle,
                                               UInt16              procId,
                                               UInt32              eventNo,
                                               Notify_CallbackFxn  cbckFxn,
                                               Void *              cbckArg);

/*!
 *  @brief  This type defines the function to send a notification event to the
 *          registered users for this notification on the specified processor.
 *          This function gets called internally from the Notify_sendEvent ()
 *          API. The Notify_sendEvent () function passes on the initialization
 *          request into the Notify driver identified by the Notify Handle.
 */
typedef Int32 (*NotifyDriver_SendEvent) (NotifyDriver_Handle handle,
                                         UInt16              procId,
                                         UInt32              eventNo,
                                         UInt32              payload,
                                         Bool                waitClear);

/*!
 *  @brief  This type defines the function to disable all events for the
 *          specified processor ID.
 *          This function gets called internally from the Notify_disable ()
 *          API. The Notify_disable () function passes on the request into the
 *          Notify driver identified by the Notify Handle.
 */
typedef UInt32 (*NotifyDriver_Disable) (NotifyDriver_Handle handle,
                                        UInt16              procId);

/*!
 *  @brief  This type defines the function to restore all events for the
 *          specified processor ID.
 *          This function gets called internally from the Notify_restore ()
 *          API. The Notify_restore () function passes on the request into the
 *          Notify driver identified by the Notify Handle.
 */
typedef Void (*NotifyDriver_Restore) (NotifyDriver_Handle handle,
                                      UInt32              key,
                                      UInt16              procId);

/*!
 *  @brief  This type defines the function to disable specified event for the
 *          specified processor ID.
 *          This function gets called internally from the Notify_disableEvent ()
 *          API. The Notify_disableEvent () function passes on the request into
 *          the Notify driver identified by the Notify Handle.
 */
typedef Void (*NotifyDriver_DisableEvent) (NotifyDriver_Handle handle,
                                           UInt16              procId,
                                           UInt32              eventNo);

/*!
 *  @brief  This type defines the function to enable specified event for the
 *          specified processor ID.
 *          This function gets called internally from the Notify_enableEvent ()
 *          API. The Notify_enableEvent () function passes on the request into
 *          the Notify driver identified by the Notify Handle.
 *
 */
typedef Void (*NotifyDriver_EnableEvent) (NotifyDriver_Handle handle,
                                          UInt16              procId,
                                          UInt32              eventNo);


/* =============================================================================
 *  Function table interface
 * =============================================================================
 */
/*!
 *  @brief  This structure defines the function table interface for the Notify
 *          driver.
 *          This function table interface must be implemented by each Notify
 *          driver and registered with the Notify module.
 *
 */
typedef struct NotifyDriver_FxnTable_tag {
    NotifyDriver_RegisterEvent   registerEvent;
    /*!< interface function registerEvent  */
    NotifyDriver_UnregisterEvent unregisterEvent;
    /*!< interface function unregisterEvent  */
    NotifyDriver_SendEvent       sendEvent;
    /*!< interface function sendEvent  */
    NotifyDriver_Disable         disable;
    /*!< interface function disable  */
    NotifyDriver_Restore         restore;
    /*!< interface function restore  */
    NotifyDriver_DisableEvent    disableEvent;
    /*!< interface function disableEvent  */
    NotifyDriver_EnableEvent     enableEvent;
    /*!< interface function enableEvent */
} NotifyDriver_FxnTable;


/* =============================================================================
 * Notify driver structure
 * =============================================================================
 */
/*!
 *  @brief  This structure defines the Notify driver object and handle used
 *          internally to contain all information required for the Notify driver
 *          This object contains all information for the Notify module to be
 *          able to identify and interact with the Notify driver.
 */
struct NotifyDriver_Object_tag {
    Notify_DriverInitStatus        isInit;
    /*!< Value of this varible indicates status of Notify driver */
    NotifyDriver_FxnTable          fxnTable;
    /*!< Function table for Notify Driver interface */
    Char                           name [NOTIFY_MAX_NAMELEN];
    /*!< Name of Notify Driver */
    NotifyDriver_Attrs             attrs;
    /*!< Notify driver attributes */
    UArg                           disableFlag [NOTIFY_MAXNESTDEPTH];
    /*!< Disable flags maintained for maximum nesting depth */
    Ptr                            driverObj;
    /*!< Pointer to driver-specific object */
};


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFYDRIVERDEFS_H_0x5f84) */
