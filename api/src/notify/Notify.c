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
 *  @file   Notify.c
 *
 *  @brief      User side Notify Manager
 *
 *  ============================================================================
 */


/* Standard headers*/
#include <Std.h>

/* Osal headers*/
#include <Trace.h>

/* Notify Headers */
#include <Notify.h>
#include <_NotifyDefs.h>
#include <NotifyDrvDefs.h>
#include <NotifyDrvUsr.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Notify Module state object
 */
typedef struct Notify_ModuleObject_tag {
    UInt32      setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} Notify_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    Notify_state
 *
 *  @brief  Notify state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
Notify_ModuleObject Notify_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the Notify module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to Notify_setup filled in by the
 *              Notify module with the default parameters. If the user
 *              does not wish to make any change in the default parameters, this
 *              API is not required to be called.
 *
 *  @param      cfg        Pointer to the Notify module configuration
 *                         structure in which the default config is to be
 *                         returned.
 *
 *  @sa         Notify_setup, NotifyDrvUsr_open, NotifyDrvUsr_ioctl,
 *              NotifyDrvUsr_close
 */
Void
Notify_getConfig (Notify_Config * cfg)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                         status = NOTIFY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    Notify_CmdArgsGetConfig    cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "Notify_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_getConfig",
                             NOTIFY_E_INVALIDARG,
                             "Argument of type (Notify_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NotifyDrvUsr_open (FALSE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            NotifyDrvUsr_ioctl (CMD_NOTIFY_GETCONFIG, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "Notify_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        NotifyDrvUsr_close (FALSE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "Notify_getConfig");
}


/*!
 *  @brief      Setup the Notify module.
 *
 *              This function sets up the Notify module. This function
 *              must be called before any other instance-level APIs can be
 *              invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then Notify_getConfig can be called to get the
 *              configuration filled with the default values. After this, only
 *              the required configuration values can be changed. If the user
 *              does not wish to make any change in the default parameters, the
 *              application can simply call Notify_setup with NULL
 *              parameters. The default parameters would get automatically used.
 *
 *  @param      cfg   Optional Notify module configuration. If provided as
 *                    NULL, default configuration is used.
 *
 *  @sa         Notify_destroy, NotifyDrvUsr_open, NotifyDrvUsr_ioctl
 */
Int32
Notify_setup (Notify_Config * cfg)
{
    Int                 status = NOTIFY_SUCCESS;
    Notify_CmdArgsSetup cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "Notify_setup", cfg);

    /* TBD: Protect from multiple threads. */
    Notify_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (Notify_state.setupRefCount > 1) {
        /*! @retval NOTIFY_S_ALREADYSETUP Success: Notify module has been
                                           already setup in this process */
        status = NOTIFY_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "    Notify_setup: Notify module has been already setup "
                   "in this process.\n"
                   "        RefCount: [%d]\n",
                   (Notify_state.setupRefCount - 1));
    }
    else {
        /* Open the driver handle. */
        status = NotifyDrvUsr_open (TRUE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
            status = NotifyDrvUsr_ioctl (CMD_NOTIFY_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "Notify_setup",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "Notify_setup", status);

    /*! @retval NOTIFY_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Destroy the Notify module.
 *
 *              Once this function is called, other Notify module APIs,
 *              except for the Notify_getConfig API cannot be called
 *              anymore.
 *
 *  @sa         Notify_setup, NotifyDrvUsr_ioctl, NotifyDrvUsr_close
 */
Int32
Notify_destroy (Void)
{
    Int                    status = NOTIFY_SUCCESS;
    Notify_CmdArgsDestroy  cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "Notify_destroy");

    /* TBD: Protect from multiple threads. */
    Notify_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (Notify_state.setupRefCount > 1) {
        /*! @retval NOTIFY_S_SETUP Success: Notify module has been setup
                                             by other clients in this process */
        status = NOTIFY_S_SETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "Notify module has been setup by other clients in this"
                   " process.\n"
                   "    RefCount: [%d]\n",
                   (Notify_state.setupRefCount + 1));
    }
    else {
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /* Close the driver handle. */
    NotifyDrvUsr_close (TRUE);

    GT_1trace (curTrace, GT_LEAVE, "Notify_destroy", status);

    /*! @retval NOTIFY_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Register a callback for a specific event with the Notify module.
 *
 *  @param      handle       Handle to the Notify Driver
 *  @param      procId       Processor Id
 *  @param      eventNo      Event number to be registered
 *  @param      cbckFxn      Callback function to be registered
 *  @param      cbckArg      Optional call back argument
 *
 *  @sa         Notify_unregisterEvent
 */
Int32
Notify_registerEvent (NotifyDriver_Handle drvHandle,
                      UInt16              procId,
                      UInt32              eventNo,
                      Notify_CallbackFxn  cbckFxn,
                      Void *              cbckArg)
{
    Int32                   status          = NOTIFY_SUCCESS;
    Notify_CommonObject *   notifyDrvHandle = (Notify_CommonObject *) drvHandle;
    Notify_CmdArgsRegisterEvent  cmdArgs;

    GT_5trace (curTrace, GT_ENTER, "Notify_registerEvent",
               drvHandle, procId, eventNo, cbckFxn, cbckArg);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));
    GT_assert (curTrace, (drvHandle != NULL));
    GT_assert (curTrace, (cbckFxn != NULL));
    /* cbckArg is optional and may be NULL. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (drvHandle == NULL) {
        status = NOTIFY_E_HANDLE;
        /*! @retval  NOTIFY_E_HANDLE Invalid NULL drvHandle provided. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_registerEvent",
                             status,
                             "Invalid NULL drvHandle provided.");
    }
    else if (cbckFxn == NULL) {
        /*! @retval  NOTIFY_E_INVALIDARG Invalid NULL cbckFxn argument
                                         provided. */
        status = NOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_registerEvent",
                             status,
                             "Invalid NULL cbckFxn provided.");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = notifyDrvHandle->knlObject;
        cmdArgs.procId = procId;
        cmdArgs.eventNo = eventNo;
        cmdArgs.fnNotifyCbck = cbckFxn;
        cmdArgs.cbckArg = cbckArg;
        cmdArgs.pid = getpid ();
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_REGISTEREVENT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_registerEvent",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Notify_registerEvent", status);

    /*! @retval NOTIFY_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Un-register the callback for the specific event with the Notify
 *              module.
 *
 *  @param      handle       Handle to the Notify Driver
 *  @param      procId       Processor Id
 *  @param      eventNo      Event number to be unregistered
 *  @param      cbckFxn      Callback function to be unregistered
 *  @param      cbckArg      Optional call back argument
 *
 *  @sa         Notify_registerEvent
 */
Int32
Notify_unregisterEvent (NotifyDriver_Handle drvHandle,
                        UInt16              procId,
                        UInt32              eventNo,
                        Notify_CallbackFxn  cbckFxn,
                        Void *              cbckArg)
{
    Int32                   status          = NOTIFY_SUCCESS;
    Notify_CommonObject *   notifyDrvHandle = (Notify_CommonObject *) drvHandle;
    Notify_CmdArgsUnregisterEvent  cmdArgs;

    GT_5trace (curTrace, GT_ENTER, "Notify_unregisterEvent",
               drvHandle, procId, eventNo, cbckFxn, cbckArg);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));
    GT_assert (curTrace, (drvHandle != NULL));
    GT_assert (curTrace, (cbckFxn != NULL));
    /* cbckArg is optional and may be NULL. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (drvHandle == NULL) {
        status = NOTIFY_E_HANDLE;
        /*! @retval  NOTIFY_E_HANDLE Invalid NULL drvHandle provided. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_unregisterEvent",
                             status,
                             "Invalid NULL drvHandle provided.");
    }
    else if (cbckFxn == NULL) {
        /*! @retval  NOTIFY_E_INVALIDARG Invalid NULL cbckFxn argument
                                         provided. */
        status = NOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_unregisterEvent",
                             status,
                             "Invalid NULL cbckFxn provided.");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = notifyDrvHandle->knlObject;
        cmdArgs.procId = procId;
        cmdArgs.eventNo = eventNo;
        cmdArgs.fnNotifyCbck = cbckFxn;
        cmdArgs.cbckArg = cbckArg;
        cmdArgs.pid = getpid ();
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_UNREGISTEREVENT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_unregisterEvent",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Notify_unregisterEvent", status);

    /*! @retval NOTIFY_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Send a notification to the specified event.
 *
 *  @param      handle      Handle to the Notify Driver
 *  @param      procId      Processor Id
 *  @param      eventNo     Event number to be sent.
 *  @param      payload     Payload to be sent alongwith the event.
 *  @param      waitClear   Indicates whether Notify driver will wait for
 *                          previous event to be cleared. If payload needs to
 *                          be sent across, this must be TRUE.
 *  @sa
 */
Int32
Notify_sendEvent (NotifyDriver_Handle drvHandle,
                  UInt16              procId,
                  UInt32              eventNo,
                  UInt32              payload,
                  Bool                waitClear)
{

    Int32                   status          = NOTIFY_SUCCESS;
    Notify_CommonObject *   notifyDrvHandle = (Notify_CommonObject *) drvHandle;
    Notify_CmdArgsSendEvent cmdArgs;

    GT_5trace (curTrace, GT_ENTER, "Notify_sendEvent",
               drvHandle, procId, eventNo, payload, waitClear);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));
    GT_assert (curTrace, (drvHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (drvHandle == NULL) {
        status = NOTIFY_E_HANDLE;
        /*! @retval  NOTIFY_E_HANDLE Invalid NULL drvHandle provided. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_sendEvent",
                             status,
                             "Invalid NULL drvHandle provided.");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = notifyDrvHandle->knlObject;
        cmdArgs.procId = procId;
        cmdArgs.eventNo = eventNo;
        cmdArgs.payload = payload;
        cmdArgs.waitClear = waitClear;
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_SENDEVENT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_sendEvent",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Notify_sendEvent", status);

    /*! @retval NOTIFY_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Disable all events for specified procId.
 *              This is equivalent to global interrupt disable for specified
 *              processor, however restricted within interrupts handled
 *              by the Notify module. All callbacks registered for all events
 *              are disabled with this API. It is not possible to disable a
 *              specific callback.
 *
 *  @param      procId       Processor Id
 *
 *  @sa         Notify_restore
 */
UInt32
Notify_disable (UInt16 procId)
{
    Int32                   status = NOTIFY_SUCCESS;
    UInt32                  key    = 0;
    Notify_CmdArgsDisable   cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "Notify_disable", procId);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));

    cmdArgs.procId = procId;
    status = NotifyDrvUsr_ioctl (CMD_NOTIFY_DISABLE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_disable",
                             status,
                             "API (through IOCTL) failed on kernel-side!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        key = cmdArgs.flags;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Notify_disable", key);

    /*! @retval flags Flags indicating disable status. */
    return (key);
}


/*!
 *  @brief      Restore the Notify module for specified procId to the state
 *              before the last Notify_disable() was called. This is equivalent
 *              to global interrupt restore, however restricted within
 *              interrupts handled by the Notify module.
 *              All callbacks registered for all events as specified in the
 *              flags are enabled with this API. It is not possible to enable a
 *              specific callback.
 *
 *  @param      key          Key received from Notify_disable
 *  @param      procId       Processor Id
 *
 *  @sa         Notify_disable
 */
Void
Notify_restore (UInt32 key, UInt16 procId)
{
    Int32                   status = NOTIFY_SUCCESS;
    Notify_CmdArgsRestore   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "Notify_restore", key, procId);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));

    cmdArgs.key = key;
    cmdArgs.procId = procId;
    status = NotifyDrvUsr_ioctl (CMD_NOTIFY_RESTORE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_restore",
                             status,
                             "API (through IOCTL) failed on kernel-side!");
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "Notify_restore");
}


/*!
 *  @brief      Disable a specific event.
 *              All callbacks registered for the specific event are disabled
 *              with this API. It is not possible to disable a specific
 *              callback.
 *
 *  @param      handle    Handle to the Notify Driver
 *  @param      procId    Processor Id
 *  @param      eventNo   Event number to be disabled
 *
 *  @sa         Notify_enableEvent
 */
Void
Notify_disableEvent (NotifyDriver_Handle drvHandle,
                     UInt16              procId,
                     UInt32              eventNo)
{
    Int32                   status          = NOTIFY_SUCCESS;
    Notify_CommonObject *   notifyDrvHandle = (Notify_CommonObject *) drvHandle;
    Notify_CmdArgsDisableEvent cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "Notify_disableEvent",
               drvHandle, procId, eventNo);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));
    GT_assert (curTrace, (drvHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (drvHandle == NULL) {
        status = NOTIFY_E_HANDLE;
        /*! @retval  NOTIFY_E_HANDLE Invalid NULL drvHandle provided. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_disableEvent",
                             status,
                             "Invalid NULL drvHandle provided.");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = notifyDrvHandle->knlObject;
        cmdArgs.procId = procId;
        cmdArgs.eventNo = eventNo;
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_DISABLEEVENT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_disableEvent",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

   GT_0trace (curTrace, GT_LEAVE, "Notify_disableEvent");
}


/*!
 *  @brief      Enable a specific event.
 *              All callbacks registered for this specific event are enabled
 *              with this API. It is not possible to enable a specific callback.
 *
 *  @param      handle    Handle to the Notify Driver
 *  @param      procId    Processor Id
 *  @param      eventNo   Event number to be enabled
 *
 *  @sa         Notify_disableEvent
 */
Void
Notify_enableEvent (NotifyDriver_Handle drvHandle,
                    UInt16              procId,
                    UInt32              eventNo)
{
    Int32                   status          = NOTIFY_SUCCESS;
    Notify_CommonObject *   notifyDrvHandle = (Notify_CommonObject *) drvHandle;
    Notify_CmdArgsEnableEvent cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "Notify_enableEvent",
               drvHandle, procId, eventNo);

    GT_assert (curTrace, (Notify_state.setupRefCount > 0));
    GT_assert (curTrace, (drvHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (drvHandle == NULL) {
        status = NOTIFY_E_HANDLE;
        /*! @retval  NOTIFY_E_HANDLE Invalid NULL drvHandle provided. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Notify_disableEvent",
                             status,
                             "Invalid NULL drvHandle provided.");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = notifyDrvHandle->knlObject;
        cmdArgs.procId = procId;
        cmdArgs.eventNo = eventNo;
        status = NotifyDrvUsr_ioctl (CMD_NOTIFY_ENABLEEVENT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "Notify_enableEvent",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

   GT_0trace (curTrace, GT_LEAVE, "Notify_enableEvent");
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
