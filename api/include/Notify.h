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
 *  @file       Notify.h
 *
 *  @brief      Notify header
 *
 *              The Notify component provides APIs to the user for sending and
 *              receiving events including short fixed-size event data between
 *              the processors. It abstracts the physical interrupts and
 *              provides multiple prioritized events over a single interrupt.
 *              The Notify component is responsible for notifying an event to
 *              its peer on the remote processor. This component shall use the
 *              services provided on the hardware platform. It provides, which
 *              are used by upper layers to establish communication amongst
 *              peers at that level.
 *  ============================================================================
 */


#if !defined (NOTIFY_H_0x5f84)
#define NOTIFY_H_0x5f84


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*!
 *  @def    NOTIFY_MODULEID
 *  @brief  Module ID for Notify.
 */
#define NOTIFY_MODULEID           (UInt16) 0x5f84


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    NOTIFY_STATUSCODEBASE
 *  @brief  Status code base for Notify module.
 */
#define NOTIFY_STATUSCODEBASE    (NOTIFY_MODULEID << 12u)

/*!
 *  @def    NOTIFY_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define NOTIFY_MAKE_FAILURE(x)    ((Int)(  0x80000000                           \
                                      | (NOTIFY_STATUSCODEBASE + (x))))

/*!
 *  @def    NOTIFY_MAKE_SUCCESS
 *
 *  @brief  Macro to make success code.
 */
#define NOTIFY_MAKE_SUCCESS(x)   (NOTIFY_STATUSCODEBASE + (x))

/*!
 *  @def    NOTIFY_E_FAIL
 *  @brief  Generic failure.
 */
#define NOTIFY_E_FAIL           NOTIFY_MAKE_FAILURE(1)

/*!
 *  @def    NOTIFY_E_TIMEOUT
 *  @brief  A timeout occurred while performing the specified operation.
 */
#define NOTIFY_E_TIMEOUT        NOTIFY_MAKE_FAILURE(2)

/*!
 *  @def    NOTIFY_E_CONFIG
 *  @brief  Configuration failure.
 */
#define NOTIFY_E_CONFIG         NOTIFY_MAKE_FAILURE(3)

/*!
 *  @def    NOTIFY_E_ALREADYINIT
 *  @brief  The module is already initialized
 */
#define NOTIFY_E_ALREADYINIT    NOTIFY_MAKE_FAILURE(4)

/*!
 *  @def    NOTIFY_E_NOTFOUND
 *  @brief  Unable to find the specified entity (e.g. registered event, driver).
 */
#define NOTIFY_E_NOTFOUND       NOTIFY_MAKE_FAILURE(5)

/*!
 *  @def    NOTIFY_E_NOTSUPPORTED
 *  @brief  The specified operation is not supported.
 */
#define NOTIFY_E_NOTSUPPORTED   NOTIFY_MAKE_FAILURE(6)

/*!
 *  @def    NOTIFY_E_INVALIDEVENT
 *  @brief  Invalid event number specified to the Notify operation.
 */
#define NOTIFY_E_INVALIDEVENT   NOTIFY_MAKE_FAILURE(7)

/*!
 *  @def    NOTIFY_E_POINTER
 *  @brief  Invalid pointer provided.
 */
#define NOTIFY_E_POINTER        NOTIFY_MAKE_FAILURE(8)
/*!
 *  @def    NOTIFY_E_RANGE
 *  @brief  The specified value is out of valid range.
 */
#define NOTIFY_E_RANGE          NOTIFY_MAKE_FAILURE(9)

/*!
 *  @def    NOTIFY_E_HANDLE
 *  @brief  An invalid handle was provided.
 */
#define NOTIFY_E_HANDLE         NOTIFY_MAKE_FAILURE(10)

/*!
 *  @def    NOTIFY_E_INVALIDARG
 *  @brief  An invalid argument was provided to the API.
 */
#define NOTIFY_E_INVALIDARG     NOTIFY_MAKE_FAILURE(11)

/*!
 *  @def    NOTIFY_E_MEMORY
 *  @brief  A memory allocation failure occurred.
 */
#define NOTIFY_E_MEMORY         NOTIFY_MAKE_FAILURE(12)

/*!
 *  @def    NOTIFY_E_INVALIDSTATE
 *  @brief  The module has not been setup.
 */
#define NOTIFY_E_INVALIDSTATE   NOTIFY_MAKE_FAILURE(13)

/*!
 *  @def    NOTIFY_E_MAXDRIVERS
 *  @brief  Maximum number of supported drivers have already been registered.
 */
#define NOTIFY_E_MAXDRIVERS     NOTIFY_MAKE_FAILURE(14)

/*!
 *  @def    NOTIFY_E_RESERVEDEVENT
 *  @brief  Invalid attempt to use a reserved event number.
 */
#define NOTIFY_E_RESERVEDEVENT  NOTIFY_MAKE_FAILURE(15)

/*!
 *  @def    NOTIFY_E_ALREADYEXISTS
 *  @brief  The specified entity (e.g. driver) already exists.
 */
#define NOTIFY_E_ALREADYEXISTS  NOTIFY_MAKE_FAILURE(16)

/*!
 *  @def    NOTIFY_E_DRIVERINIT
 *  @brief  The Notify driver has not been initialized.
 */
#define NOTIFY_E_DRIVERINIT     NOTIFY_MAKE_FAILURE(17)

/*!
 *  @def    NOTIFY_E_NOTREADY
 *  @brief  The remote processor is not ready to receive the event.
 */
#define NOTIFY_E_NOTREADY       NOTIFY_MAKE_FAILURE(18)

/*!
 *  @def    NOTIFY_E_REGDRVFAILED
 *  @brief  Failed to register driver with Notify module.
 */
#define NOTIFY_E_REGDRVFAILED  NOTIFY_MAKE_FAILURE(19)

/*!
 *  @def    NOTIFY_E_REGDRVFAILED
 *  @brief  Failed to unregister driver with Notify module.
 */
#define NOTIFY_E_UNREGDRVFAILED NOTIFY_MAKE_FAILURE(20)

/*!
 *  @def    NOTIFY_E_OSFAILURE
 *  @brief  Failure in an OS-specific operation.
 */
#define NOTIFY_E_OSFAILURE      NOTIFY_MAKE_FAILURE(21)

/*!
 *  @def    NOTIFY_E_MAXEVENTS
 *  @brief  Maximum number of supported events have already been registered.
 */
#define NOTIFY_E_MAXEVENTS      NOTIFY_MAKE_FAILURE(22)

/*!
 *  @def    NOTIFY_E_MAXCLIENTS
 *  @brief  Maximum number of supported user clients have already been
 *          registered.
 */
#define NOTIFY_E_MAXCLIENTS     NOTIFY_MAKE_FAILURE(23)

/*!
 *  @def    NOTIFY_SUCCESS
 *  @brief  Operation is successful.
 */
#define NOTIFY_SUCCESS          NOTIFY_MAKE_SUCCESS(0)

/*!
 *  @def    NOTIFY_S_ALREADYSETUP
 *  @brief  The ProcMgr module has already been setup in this process.
 */
#define NOTIFY_S_ALREADYSETUP   NOTIFY_MAKE_SUCCESS(1)

/*!
 *  @def    NOTIFY_S_OPENHANDLE
 *  @brief  Other ProcMgr clients have still setup the ProcMgr module.
 */
#define NOTIFY_S_SETUP          NOTIFY_MAKE_SUCCESS(2)

/*!
 *  @def    NOTIFY_S_OPENHANDLE
 *  @brief  Other ProcMgr handles are still open in this process.
 */
#define NOTIFY_S_OPENHANDLE     NOTIFY_MAKE_SUCCESS(3)

/*!
 *  @def    NOTIFY_S_ALREADYEXISTS
 *  @brief  The ProcMgr instance has already been created/opened in this process
 */
#define NOTIFY_S_ALREADYEXISTS  NOTIFY_MAKE_SUCCESS(4)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @def    NOTIFY_MAX_DRIVERS
 *  @brief  Maximum number of Notify drivers supported.
 */
#define NOTIFY_MAX_DRIVERS   16u

/*!
 *  @def    NOTIFY_MAX_NAMELEN
 *  @brief  Maximum length of the name of Notify drivers, inclusive of NULL
 *          string terminator.
 */
#define NOTIFY_MAX_NAMELEN   32u

/*!
 *  @def    NOTIFY_MAXNESTDEPTH
 *  @brief  Maximum depth for nesting Notify_disable / Notify_restore calls.
 */
#define NOTIFY_MAXNESTDEPTH  2u

/*!
 *  @def    NOTIFY_SYSTEM_KEY
 *  @brief  This key must be provided as the upper 16 bits of the eventNo when
 *          registering for an event, if any reserved event numbers are to be
 *          used.
 */
#define NOTIFY_SYSTEM_KEY    (UInt16) 0xC1D2


/* Forward declaration of NotifyDriver_Object */
typedef struct NotifyDriver_Object_tag NotifyDriver_Object;

/*!
 *  @brief  Defines the type for the handle to the Notify driver.
 */
typedef NotifyDriver_Object * NotifyDriver_Handle;


/*!
 *  @brief   Module configuration structure.
 *           This structure defines attributes for initialization of the Notify
 *           module.
 */
typedef struct Notify_Config_tag {
    UInt32   maxDrivers;
    /*!< Maximum number of drivers that can be created for Notify at a time. */
} Notify_Config ;

/*!
 *  @brief   Signature of the callback function to be registered with the Notify
 *           component.
 */
typedef Void (*Notify_CallbackFxn) (UInt16    procId,
                                    UInt32    eventNo,
                                    Ptr       arg,
                                    UInt32    payload);


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get the default configuration for the Notify module. */
Void Notify_getConfig (Notify_Config * cfg);

/* Function to setup the Notify Module */
Int32 Notify_setup (Notify_Config * cfg);

/* Function to destroy the Notify module */
Int32 Notify_destroy (Void);

/* Function to register an event */
Int32 Notify_registerEvent (NotifyDriver_Handle drvHandle,
                            UInt16              procId,
                            UInt32              eventno,
                            Notify_CallbackFxn  cbckFxn,
                            Void *              cbckArg);

/* Function to unregister an event */
Int32 Notify_unregisterEvent (NotifyDriver_Handle handle,
                              UInt16              procId,
                              UInt32              eventno,
                              Notify_CallbackFxn  cbckFxn,
                              Void *              cbckArg);

/* Function to send an event to other processor */
Int32  Notify_sendEvent (NotifyDriver_Handle handle ,
                         UInt16              procId,
                         UInt32              eventno,
                         UInt32              payload,
                         Bool                waitClear);

/* Function to disable Notify module */
UInt32 Notify_disable (UInt16 procId);

/* Function to restore Notify module state */
Void Notify_restore (UInt32 key,
                     UInt16 procId);

/* Function to disable particular event */
Void Notify_disableEvent (NotifyDriver_Handle handle,
                          UInt16              procId,
                          UInt32              eventno);

/* Function to enable particular event */
Void Notify_enableEvent (NotifyDriver_Handle handle,
                         UInt16              procId,
                         UInt32              eventNo);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFY_H_0x5f84) */
