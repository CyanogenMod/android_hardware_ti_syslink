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
 *  @file       NotifyDriverShm.h
 *
 *  @brief      Notify shared memory driver header
 *
 *
 */


#if !defined (NOTIFYDRIVERSHM_H_0xb9d4)
#define NOTIFYDRIVERSHM_H_0xb9d4


/* Module headers. */
#include <Notify.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*!
 *  @def    NOTIFYDRIVERSHM_MODULEID
 *  @brief  Module ID for NotifyDriverShm.
 */
#define NOTIFYDRIVERSHM_MODULEID           (UInt16) 0xb9d4


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    NOTIFYDRIVERSHM_STATUSCODEBASE
 *  @brief  Status code base for NotifyDriverShm module.
 */
#define NOTIFYDRIVERSHM_STATUSCODEBASE    (NOTIFYDRIVERSHM_MODULEID << 12u)

/*!
 *  @def    NOTIFYDRIVERSHM_MAKE_FAILURE
 *  @brief  Macro to make failure code.
 */
#define NOTIFYDRIVERSHM_MAKE_FAILURE(x)    ((Int)(  0x80000000           \
                                     | (NOTIFYDRIVERSHM_STATUSCODEBASE + (x))))

/*!
 *  @def    NOTIFYDRIVERSHM_MAKE_SUCCESS
 *
 *  @brief  Macro to make success code.
 */
#define NOTIFYDRIVERSHM_MAKE_SUCCESS(x)   (NOTIFYDRIVERSHM_STATUSCODEBASE + (x))

/*!
 *  @def    NOTIFYDRIVERSHM_E__FAIL
 *  @brief  Generic failure.
 */
#define NOTIFYDRIVERSHM_E_FAIL           NOTIFYDRIVERSHM_MAKE_FAILURE(1)

/*!
 *  @def    NOTIFYDRIVERSHM_E__HANDLE
 *  @brief  An invalid handle was provided.
 */
#define NOTIFYDRIVERSHM_E_HANDLE         NOTIFYDRIVERSHM_MAKE_FAILURE(2)

/*!
 *  @def    NOTIFYDRIVERSHM_E__INVALIDARG
 *  @brief  An invalid argument was provided to the API.
 */
#define NOTIFYDRIVERSHM_E_INVALIDARG     NOTIFYDRIVERSHM_MAKE_FAILURE(3)

/*!
 *  @def    NOTIFYDRIVERSHM_E__MEMORY
 *  @brief  A memory allocation failure occurred.
 */
#define NOTIFYDRIVERSHM_E_MEMORY         NOTIFYDRIVERSHM_MAKE_FAILURE(4)

/*!
 *  @def    NOTIFYDRIVERSHM_E_NOTFOUND
 *  @brief  Unable to find the specified entity (e.g. registered event, driver).
 */
#define NOTIFYDRIVERSHM_E_NOTFOUND       NOTIFYDRIVERSHM_MAKE_FAILURE(5)

/*!
 *  @def    NOTIFYDRIVERSHM_E_OSFAILURE
 *  @brief  Failure in an OS-specific operation.
 */
#define NOTIFYDRIVERSHM_E_OSFAILURE      NOTIFYDRIVERSHM_MAKE_FAILURE(6)

/*!
 *  @def    NOTIFYDRIVERSHM_E_CONFIG
 *  @brief  Configuration failure.
 */
#define NOTIFYDRIVERSHM_E_CONFIG         NOTIFYDRIVERSHM_MAKE_FAILURE(7)

/*!
 *  @def    NOTIFYDRIVERSHM_E_DRIVERINIT
 *  @brief  The NotifyDriverShm driver has not been initialized.
 */
#define NOTIFYDRIVERSHM_E_DRIVERINIT     NOTIFYDRIVERSHM_MAKE_FAILURE(8)

/*!
 *  @def    NOTIFYDRIVERSHM_E_NOTREADY
 *  @brief  The remote processor is not ready to receive the event.
 */
#define NOTIFYDRIVERSHM_E_NOTREADY       NOTIFYDRIVERSHM_MAKE_FAILURE(9)

/*!
 *  @def    NOTIFYDRIVERSHM_E_TIMEOUT
 *  @brief  A timeout occurred while performing the specified operation.
 */
#define NOTIFYDRIVERSHM_E_TIMEOUT        NOTIFYDRIVERSHM_MAKE_FAILURE(10)

/*!
 *  @def    NOTIFYDRIVERSHM_E_ACCESSDENIED
 *  @brief  The operation is not permitted in this process.
 */
#define NOTIFYDRIVERSHM_E_ACCESSDENIED   NOTIFYDRIVERSHM_MAKE_FAILURE(11)

/*!
 *  @def    NOTIFYDRIVERSHM_SUCCESS
 *  @brief  Operation is successful.
 */
#define NOTIFYDRIVERSHM_SUCCESS          NOTIFYDRIVERSHM_MAKE_SUCCESS(0)

/*!
 *  @def    NOTIFYDRIVERSHM_S_ALREADYSETUP
 *  @brief  The NotifyDriverShm module has already been setup in this process.
 */
#define NOTIFYDRIVERSHM_S_ALREADYSETUP   NOTIFYDRIVERSHM_MAKE_SUCCESS(1)

/*!
 *  @def    NOTIFYDRIVERSHM_S_OPENHANDLE
 *  @brief  Other NotifyDriverShm clients have still setup the NotifyDriverShm
 *          module.
 */
#define NOTIFYDRIVERSHM_S_SETUP          NOTIFYDRIVERSHM_MAKE_SUCCESS(2)

/*!
 *  @def    NOTIFYDRIVERSHM_S_OPENHANDLE
 *  @brief  Other NotifyDriverShm handles are still open in this process.
 */
#define NOTIFYDRIVERSHM_S_OPENHANDLE     NOTIFYDRIVERSHM_MAKE_SUCCESS(3)

/*!
 *  @def    NOTIFYDRIVERSHM_S_ALREADYEXISTS
 *  @brief  The NotifyDriverShm instance has already been created/opened in this
 *          process
 */
#define NOTIFYDRIVERSHM_S_ALREADYEXISTS  NOTIFYDRIVERSHM_MAKE_SUCCESS(4)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Module configuration structure.
 */
typedef struct NotifyDriverShm_Config {
    Handle gateHandle;
    /*!< Handle of gate to be used for local thread safety. If provided as NULL,
         gate handle is created internally. */
} NotifyDriverShm_Config;

/*!
 *  @brief   This structure defines the configuration structure for
 *           initialization of the Notify driver.
 */
typedef struct NotifyDriverShm_Params_tag {
    UInt32    sharedAddr;
    /*!< Shared memory base address */
    UInt32    sharedAddrSize;
    /*!< Shared memory size required for shared structures */
    UInt32    numEvents;
    /*!< Number of events to be supported by driver */
    UInt32    recvIntId;
    /*!< Hardware interrupt ID on which interrupts are to be received. */
    UInt32    sendIntId;
    /*!< Hardware interrupt ID to which interrupts are to be sent. */
    UInt32    remoteProcId;
    /*!< Processor Id of remote processor required for communication */
    UInt32    numReservedEvents;
    /*!< Number of reserved events to be supported by driver */
    UInt32    sendEventPollCount;
    /*!< Poll count for which the sendEvent will spin waiting for the remote
         processor to clear the event. If specified as -1, the driver will
         spin forever. */
} NotifyDriverShm_Params;


/*!
 *  @brief  Defines the type for the handle to the NotifyDriverShm
 */
typedef NotifyDriver_Handle NotifyDriverShm_Handle;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get the default configuration for the NotifyDriverShm module. */
Void NotifyDriverShm_getConfig (NotifyDriverShm_Config * cfg);

/* Function to setup the NotifyDriverShm module. */
Int NotifyDriverShm_setup (NotifyDriverShm_Config * cfg);

/* Function to destroy the NotifyDriverShm module. */
Int NotifyDriverShm_destroy (Void);

/* Get the default parameters for the NotifyShmDriver. */
Void NotifyDriverShm_Params_init (NotifyDriverShm_Handle   handle,
                                  NotifyDriverShm_Params * params);

/* Create an instance of the NotifyShmDriver. */
NotifyDriverShm_Handle
NotifyDriverShm_create (      String                   driverName,
                        const NotifyDriverShm_Params * params);

/* Function to exit from Notify driver */
Int32 NotifyDriverShm_delete (NotifyDriver_Handle * handle);

/* Function to open a previously created instance */
Int NotifyDriverShm_open   (String                   driverName,
                            NotifyDriverShm_Handle * handlePtr);

/* Function to close a previously opened instance */
Int NotifyDriverShm_close  (NotifyDriverShm_Handle * handlePtr);

/* Get the shared memory requirements for the NotifyDriverShm. */
UInt32 NotifyDriverShm_sharedMemReq (const NotifyDriverShm_Params * params);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFYDRIVERSHM_H_0xb9d4) */

