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
 *  @file   NameServerRemoteNotify.h
 *
 *  @brief      NameServer Remote Transport
 *
 *              This module provides functionality to get name value pair from a
 *              remote nameserver. it uses notify to send packet containing data
 *              to the remote processor and then waits on a semaphore. Other
 *              processor on getting a notify event, finds the name value pair
 *              in the local nameservers. if it find the pair then it sends out
 *              a notify event with necessary packet to the other processor.<br>
 *              Modules specific name value pair are distinguished by looking
 *              into the name field, name field would be <module_name>:name type
 *              . So first, remote processor would get the module specific
 *              nameserver handle from the nameserver_modules nameserver by
 *              providing <module_name>, once it gets the module specific
 *              nameserver handle, it uses the name to find out the pair and
 *              sends it back.
 *  ============================================================================
 */


#ifndef NAMESERVERREMOTENOTIFY_H_0X5711
#define NAMESERVERREMOTENOTIFY_H_0X5711


/* Module level headers */
#include <SharedRegion.h>
#include <Notify.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    NAMESERVERREMOTENOTIFY_MODULEID
 *  @brief  Unique module ID.
 */
#define NAMESERVERREMOTENOTIFY_MODULEID      (0x08FD)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    NAMESERVERREMOTENOTIFY_ERRORCODEBASE
 *  @brief  Error code base for NAMESERVERRT.
 */
#define NAMESERVERREMOTENOTIFY_ERRORCODEBASE                                   \
                                        (NAMESERVERREMOTENOTIFY_MODULEID << 12u)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define NAMESERVERREMOTENOTIFY_MAKE_FAILURE(x)                                 \
                             ((Int) (  0x80000000                              \
                                     | (  NAMESERVERREMOTENOTIFY_ERRORCODEBASE \
                                        + (x))))

/*!
 *  @def    NAMESERVERREMOTENOTIFY_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define NAMESERVERREMOTENOTIFY_MAKE_SUCCESS(x)                                 \
                                    (NAMESERVERREMOTENOTIFY_ERRORCODEBASE + (x))

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define NAMESERVERREMOTENOTIFY_E_INVALIDARG                                    \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(1)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define NAMESERVERREMOTENOTIFY_E_MEMORY                                        \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(2)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_BUSY
 *  @brief  The name is already registered or not.
 */
#define NAMESERVERREMOTENOTIFY_E_BUSY                                          \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(3)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_FAIL
 *  @brief  Generic failure.
 */
#define NAMESERVERREMOTENOTIFY_E_FAIL                                          \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(4)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_NOTFOUND
 *  @brief  Name not found in the SharedRegion.
 */
#define NAMESERVERREMOTENOTIFY_E_NOTFOUND                                      \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(5)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_ALREADYEXIST
 *  @brief  Entry already exists.
 */
#define NAMESERVERREMOTENOTIFY_E_ALREADYEXIST                                 \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(6)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_INVALIDSTATE
 *  @brief  Module is in invalid state.
 */
#define NAMESERVERREMOTENOTIFY_E_INVALIDSTATE                                  \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(7)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_OVERLAP
 *  @brief  Entries overlaps.
 */
#define NAMESERVERREMOTENOTIFY_E_OVERLAP                                       \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(8)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_HANDLE
 *  @brief  Invalid handle provided.
 */
#define NAMESERVERREMOTENOTIFY_E_HANDLE                                       \
                                          NAMESERVERREMOTENOTIFY_MAKE_FAILURE(9)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define NAMESERVERREMOTENOTIFY_E_OSFAILURE    \
                                        NAMESERVERREMOTENOTIFY_MAKE_FAILURE(10)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_SUCCESS
 *  @brief  Operation successful.
 */
#define NAMESERVERREMOTENOTIFY_SUCCESS                                         \
                                          NAMESERVERREMOTENOTIFY_MAKE_SUCCESS(0)

/*!
 *  @def    NAMESERVERREMOTENOTIFY_S_ALREADYSETUP
 *  @brief  The NAMESERVERREMOTENOTIFY module has
 *          already been setup in this process.
 */
#define NAMESERVERREMOTENOTIFY_S_ALREADYSETUP \
                                          NAMESERVERREMOTENOTIFY_MAKE_SUCCESS(1)

/* =============================================================================
 * Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Module configuration structure.
 */
typedef struct NameServerRemoteNotify_Config {
    UInt32 reserved;
    /*!< Reserved value (not currently used) */
} NameServerRemoteNotify_Config;

/*!
 *  @brief  Module configuration structure.
 */
typedef struct NameServerRemoteNotify_Params {
    UInt32              notifyEventNo;
    /*!< Notify event number */
    NotifyDriver_Handle notifyDriver;
    /*!< Notify Driver handle */
    Ptr                 sharedAddr;
    /*!< Address of the shared memory */
    UInt32              sharedAddrSize;
    /*!< Size of the shared memory */
    Gate_Handle         gate;
    /*!< Handle to the gate used for protecting nameserver add and delete */
} NameServerRemoteNotify_Params;


/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */
/*! @brief forward declaration of NameServerRemoteNotify_Handle */
typedef struct NameServerRemoteNotify_Object * NameServerRemoteNotify_Handle;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get the default configuration for the NameServerRemoteNotify
 * module.
 */
Void NameServerRemoteNotify_getConfig (NameServerRemoteNotify_Config * cfg);

/* Function to setup the NameServerRemoteNotify module. */
Int NameServerRemoteNotify_setup (NameServerRemoteNotify_Config * cfg);

/* Function to destroy the NameServerRemoteNotify module. */
Int NameServerRemoteNotify_destroy (Void);

/* Function to get the current configuration values. */
Void
NameServerRemoteNotify_Params_init (NameServerRemoteNotify_Handle   handle,
                                    NameServerRemoteNotify_Params * params);

/* Function to setup the Name Server remote notify. */
NameServerRemoteNotify_Handle
NameServerRemoteNotify_create (      UInt16                          procId,
                               const NameServerRemoteNotify_Params * params);

/* Function to destroy the Name Server remote notify. */
Int
NameServerRemoteNotify_delete (NameServerRemoteNotify_Handle * instp);


/* Function to get a name/value from remote nameserver. */
Int
NameServerRemoteNotify_get (NameServerRemoteNotify_Handle   handle,
                            String                          instanceName,
                            String                          name,
                            Ptr                             value,
                            Int                             valueLen,
                            Ptr                             reserved);

/* Get the shared memory requirements for the NameServerRemoteNotify. */
UInt32
NameServerRemoteNotify_sharedMemReq (
                                const NameServerRemoteNotify_Params * params);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* NAMESERVERREMOTENOTIFY_H */
