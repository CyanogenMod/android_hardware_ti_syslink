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
 *  @file   GatePeterson.h
 *
 *  @brief      Defines for Gate based on Peterson Algorithm.
 *
 *              This module implements the Peterson Algorithm for mutual
 *              exclusion of shared memory. This implementation works for only 2
 *              processors.<br>
 *              This module is instance based. Each instance requires a small
 *              piece of shared memory. This is specified via the sharedAddr
 *              parameter to the create. The proper sharedAddrSize parameter
 *              can be determined via the #GatePeterson_sharedMemReq call. Note:
 *              the parameters to this function must be the same that will used
 *              to create or open the instance.<br>
 *              The GatePeterson module uses a NameServer instance
 *              to store instance information when an instance is created and
 *              the name parameter is non-NULL. If a name is supplied, it must
 *              be unique for all GatePeterson instances.
 *              The #GatePeterson_create also initializes the shared memory as
 *              needed. The shared memory must be initialized to 0 or all ones
 *              (e.g. 0xFFFFFFFFF) before the GatePeterson instance is created.
 *              Once an instance is created, an open can be performed. The
 *              #GatePeterson_open is used to gain access to the same
 *              GatePeterson instance. Generally an instance is created on one
 *              processor and opened on the other processor.<br>
 *              The open returns a GatePeterson instance handle like the create,
 *              however the open does not modify the shared memory. Generally an
 *              instance is created on one processor and opened on the other
 *              processor.<br>
 *              There are two options when opening the instance:<br>
 *              - Supply the same name as specified in the create. The
 *              GatePeterson module queries the NameServer to get the needed
 *              information.<br>
 *              - Supply the same sharedAddr value as specified in the create.
 *              If the open is called before the instance is created, open
 *              returns NULL.<br>
 *              There is currently a list of restrictions for the module:<br>
 *              - Both processors must have the same endianness. Endianness
 *             conversion may supported in a future version of GatePeterson.<br>
 *              - The module will be made a gated module
 *
 *  ============================================================================
 */


#ifndef GATEPETERSON_H_0xF415
#define GATEPETERSON_H_0xF415

/* Utilities & Osal headers */
#include <Gate.h>

#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    GATEPETERSON_MODULEID
 *  @brief  Unique module ID.
 */
#define GATEPETERSON_MODULEID       (0xF415)

/* =============================================================================
 * Module Success and Failure codes
 * =============================================================================
 */
/*!
 *  @def    GATEPETERSON_STATUSCODEBASE
 *  @brief  Error code base for GatePeterson.
 */
#define GATEPETERSON_STATUSCODEBASE  (GATEPETERSON_MODULEID << 12u)

/*!
 *  @def    GATEPETERSON_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define GATEPETERSON_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                               + (GATEPETERSON_STATUSCODEBASE  \
                                               + (x))))

/*!
 *  @def    GATEPETERSON_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define GATEPETERSON_MAKE_SUCCESS(x)    (GATEPETERSON_STATUSCODEBASE + (x))

/*!
 *  @def    GATEPETERSON_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define GATEPETERSON_E_INVALIDARG       GATEPETERSON_MAKE_FAILURE(1)

/*!
 *  @def    GATEPETERSON_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define GATEPETERSON_E_MEMORY           GATEPETERSON_MAKE_FAILURE(2)

/*!
 *  @def    GATEPETERSON_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define GATEPETERSON_E_BUSY             GATEPETERSON_MAKE_FAILURE(3)

/*!
 *  @def    GATEPETERSON_E_FAIL
 *  @brief  Generic failure.
 */
#define GATEPETERSON_E_FAIL             GATEPETERSON_MAKE_FAILURE(4)

/*!
 *  @def    GATEPETERSON_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define GATEPETERSON_E_NOTFOUND         GATEPETERSON_MAKE_FAILURE(5)

/*!
 *  @def    GATEPETERSON_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define GATEPETERSON_E_INVALIDSTATE     GATEPETERSON_MAKE_FAILURE(6)

/*!
 *  @def    GATEPETERSON_E_NOTONWER
 *  @brief  Instance is not created on this processor.
 */
#define GATEPETERSON_E_NOTONWER         GATEPETERSON_MAKE_FAILURE(7)

/*!
 *  @def    GATEPETERSON_E_REMOTEACTIVE
 *  @brief  Remote opener of the instance has not closed the instance.
 */
#define GATEPETERSON_E_REMOTEACTIVE     GATEPETERSON_MAKE_FAILURE(8)

/*!
 *  @def    GATEPETERSON_E_INUSE
 *  @brief  Indicates that the instance is in use..
 */
#define GATEPETERSON_E_INUSE            GATEPETERSON_MAKE_FAILURE(9)

/*!
 *  @def    GATEPETERSON_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define GATEPETERSON_E_OSFAILURE        GATEPETERSON_MAKE_FAILURE(0xA)

/*!
 *  @def    GATEPETERSON_E_VERSION
 *  @brief  Version mismatch error.
 */
#define GATEPETERSON_E_VERSION          GATEPETERSON_MAKE_FAILURE(0xB)

/*!
 *  @def    GATEPETERSON_SUCCESS
 *  @brief  Operation successful.
 */
#define GATEPETERSON_SUCCESS            GATEPETERSON_MAKE_SUCCESS(0)

/*!
 *  @def    GATEPETERSON_S_ALREADYSETUP
 *  @brief  The GATEPETERSON module has already been setup in this process.
 */
#define GATEPETERSON_S_ALREADYSETUP     GATEPETERSON_MAKE_SUCCESS(1)


/* =============================================================================
 * Macros
 * =============================================================================
 */
/*! @brief Forward declaration of structure defining object for the
 *                 Gate Peterson. */
#define  GatePeterson_Object Gate_Object

/*!
 *  @brief  Handle for the GatePeterson.
 */
#define  GatePeterson_Handle Gate_Handle

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  A set of context protection levels that each correspond to single
 *          processor gates used for local protection.
 */
typedef enum GatePeterson_Protect {
    GatePeterson_Protect_DEFAULT   = 0u,
    /*!< Default protection level. */
    GatePeterson_Protect_NONE      = 1u,
    /*!< No local protection required. */
    GatePeterson_Protect_INTERRUPT = 2u,
    /*!< Interrupt-level protection required. */
    GatePeterson_Protect_TASKLET   = 3u,
    /*!< Tasklet level protection required. */
    GatePeterson_Protect_THREAD    = 4u,
    /*!< Thread level protection required. */
    GatePeterson_Protect_PROCESS   = 5u,
    /*!< Process level protection required. */
    GatePeterson_Protect_EndValue  = 6u
    /*!< End delimiter indicating start of invalid values for this enum */
} GatePeterson_Protect;

/*!
 *  @brief  Structure defining config parameters for the Gate Peterson module.
 */
typedef struct GatePeterson_Config {
    GatePeterson_Protect defaultProtection;
    /*!< Default module-wide local context protection level. The level of
     * protection specified here determines which local gate is created per
     * GatePeterson instance for local protection during create. The instance
     * configuration parameter may be used to override this module setting per
     * instance.  The configuration used here should reflect both the context
     * in which enter and leave are to be called, as well as the maximum level
     * of protection needed locally.
     */
    UInt32               maxNameLen;
    /*!< Maximum length of GP name */
    Bool                 useNameServer;
    /*!< Whether to have this module use the NameServer or not. If the
     *   NameServer is not needed, set this configuration parameter to false.
     *   This informs GatePeterson not to pull in the NameServer module. In this
     *   case, all names passed into create and open are ignored.
     */
} GatePeterson_Config;

/*!
 *  @brief  Structure defining config parameters for the Gate Peterson
 *          instances.
 */
typedef struct GatePeterson_Params {
    Ptr                  sharedAddr;
    /*!< Address of the shared memory. The creator must supply a cache-aligned
     *   address in shared memory that will be used to store shared state
     *   information.
     */
    UInt32               sharedAddrSize;
    /*!< Size of the shared memory region. Can use GatePeterson_sharedMemReq
     *   call to determine the required size.
     */
    String               name;
    /*!< If using NameServer, name of this instance. The name (if not NULL) must
     *   be unique among all GatePeterson instances in the entire system.
     */
    GatePeterson_Protect localProtection;
    /*!< Local gate protection level. The default value, (Protect_DEFAULT)
     *   results in inheritance from module-level defaultProtection. This
     *   instance setting should be set to an alternative only if a different
     * local protection level is needed for the instance.
     */
    Bool                 useNameServer;
    /*!< Whether to have this module use the NameServer or not. If the
     *   NameServer is not needed, set this configuration parameter to false.
     *   This informs GatePeterson not to pull in the NameServer module. In this
     *   case, all names passed into create and open are ignored.
     */
} GatePeterson_Params;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function to get the default configuration for the GatePeterson module. */
Void
GatePeterson_getConfig (GatePeterson_Config * config);

/* Function to setup the GatePeterson module. */
Int
GatePeterson_setup (const GatePeterson_Config * config);

/* Function to destroy the GatePeterson module */
Int
GatePeterson_destroy (Void);

/* Get the default parameters for the GatePeterson instance. */
Void
GatePeterson_Params_init (GatePeterson_Handle   handle,
                          GatePeterson_Params * params);

/* Function to create an instance of GatePeterson */
GatePeterson_Handle
GatePeterson_create (const GatePeterson_Params * params);

/* Function to delete an instance of GatePeterson */
Int
GatePeterson_delete (GatePeterson_Handle * handlePtr);

/* Function to open a previously created GatePeterson instance.  */
Int
GatePeterson_open   (GatePeterson_Handle * handlePtr,
                     GatePeterson_Params * params);

/* Function to close a previously opened instance */
Int
GatePeterson_close  (GatePeterson_Handle * handlePtr);

/* Function to enter the GatePeterson instance */
UInt32
GatePeterson_enter  (GatePeterson_Handle handle);

/* Function to leave the GatePeterson instance */
Void
GatePeterson_leave  (GatePeterson_Handle handle, UInt32 key);

/* Function to return the shared memory requirement for a single instance */
UInt32
GatePeterson_sharedMemReq (const GatePeterson_Params * params);


/* =============================================================================
 * Internal functions
 * =============================================================================
 */
/* Returns the GatePeterson kernel object pointer. */
Void *
GatePeterson_getKnlHandle (GatePeterson_Handle handle);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* GATEPETERSON_H_0xF415 */
