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
 *  @file   GateHWSpinLock.h
 *
 *  @brief      Defines for Gate based on Hardware SpinLock.
 *
 *  ============================================================================
 */


#ifndef GATEHWSPINLOCK_H_0xF416
#define GATEHWSPINLOCK_H_0xF416

/* Utilities & Osal headers */
#include <Gate.h>

#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    GATEHWSPINLOCK_MODULEID
 *  @brief  Unique module ID.
 */
#define GATEHWSPINLOCK_MODULEID       (0xF416)

/* =============================================================================
 * Module Success and Failure codes
 * =============================================================================
 */
/*!
 *  @def    GATEHWSPINLOCK_STATUSCODEBASE
 *  @brief  Error code base for GateHWSpinLock.
 */
#define GATEHWSPINLOCK_STATUSCODEBASE  (GATEHWSPINLOCK_MODULEID << 12u)

/*!
 *  @def    GATEHWSPINLOCK_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define GATEHWSPINLOCK_MAKE_FAILURE(x)    ((Int) ( 0x80000000 + \
                                               (GATEHWSPINLOCK_STATUSCODEBASE \
                                               + (x))))

/*!
 *  @def    GATEHWSPINLOCK_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define GATEHWSPINLOCK_MAKE_SUCCESS(x)    (GATEHWSPINLOCK_STATUSCODEBASE + (x))

/*!
 *  @def    GATEHWSPINLOCK_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define GATEHWSPINLOCK_E_INVALIDARG       GATEHWSPINLOCK_MAKE_FAILURE(1)

/*!
 *  @def    GATEHWSPINLOCK_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define GATEHWSPINLOCK_E_MEMORY           GATEHWSPINLOCK_MAKE_FAILURE(2)

/*!
 *  @def    GATEHWSPINLOCK_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define GATEHWSPINLOCK_E_BUSY             GATEHWSPINLOCK_MAKE_FAILURE(3)

/*!
 *  @def    GATEHWSPINLOCK_E_FAIL
 *  @brief  Generic failure.
 */
#define GATEHWSPINLOCK_E_FAIL             GATEHWSPINLOCK_MAKE_FAILURE(4)

/*!
 *  @def    GATEHWSPINLOCK_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define GATEHWSPINLOCK_E_NOTFOUND         GATEHWSPINLOCK_MAKE_FAILURE(5)

/*!
 *  @def    GATEHWSPINLOCK_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define GATEHWSPINLOCK_E_INVALIDSTATE     GATEHWSPINLOCK_MAKE_FAILURE(6)

/*!
 *  @def    GATEHWSPINLOCK_E_NOTONWER
 *  @brief  Instance is not created on this processor.
 */
#define GATEHWSPINLOCK_E_NOTONWER         GATEHWSPINLOCK_MAKE_FAILURE(7)

/*!
 *  @def    GATEHWSPINLOCK_E_REMOTEACTIVE
 *  @brief  Remote opener of the instance has not closed the instance.
 */
#define GATEHWSPINLOCK_E_REMOTEACTIVE     GATEHWSPINLOCK_MAKE_FAILURE(8)

/*!
 *  @def    GATEHWSPINLOCK_E_INUSE
 *  @brief  Indicates that the instance is in use..
 */
#define GATEHWSPINLOCK_E_INUSE            GATEHWSPINLOCK_MAKE_FAILURE(0xA)

/*!
 *  @def    GATEHWSPINLOCK_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define GATEHWSPINLOCK_E_OSFAILURE        GATEHWSPINLOCK_MAKE_FAILURE(0xB)

/*!
 *  @def    GATEHWSPINLOCK_E_VERSION
 *  @brief  Version mismatch error.
 */
#define GATEHWSPINLOCK_E_VERSION          GATEHWSPINLOCK_MAKE_FAILURE(0xC)

/*!
 *  @def    GATEHWSPINLOCK_SUCCESS
 *  @brief  Operation successful.
 */
#define GATEHWSPINLOCK_SUCCESS            GATEHWSPINLOCK_MAKE_SUCCESS(0)

/*!
 *  @def    GATEHWSPINLOCK_S_ALREADYSETUP
 *  @brief  The GATEHWSPINLOCK module has already been setup in this process.
 */
#define GATEHWSPINLOCK_S_ALREADYSETUP     GATEHWSPINLOCK_MAKE_SUCCESS(1)


/* =============================================================================
 * Macros
 * =============================================================================
 */
/*! @brief Forward declaration of structure defining object for the
 *                 GateHWSpinLock. */
#define  GateHWSpinLock_Object Gate_Object

/*!
 *  @brief  Handle for the GateHWSpinLock.
 */
#define  GateHWSpinLock_Handle Gate_Handle

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  A set of context protection levels that each correspond to single
 *          processor gates used for local protection.
 */
typedef enum GateHWSpinLock_Protect {
    GateHWSpinLock_Protect_DEFAULT   = 0u,
    /*!< Default protection level. */
    GateHWSpinLock_Protect_NONE      = 1u,
    /*!< No local protection required. */
    GateHWSpinLock_Protect_INTERRUPT = 2u,
    /*!< Interrupt-level protection required. */
    GateHWSpinLock_Protect_TASKLET   = 3u,
    /*!< Tasklet level protection required. */
    GateHWSpinLock_Protect_THREAD    = 4u,
    /*!< Thread level protection required. */
    GateHWSpinLock_Protect_PROCESS   = 5u,
    /*!< Process level protection required. */
    GateHWSpinLock_Protect_EndValue  = 6u
    /*!< End delimiter indicating start of invalid values for this enum */
} GateHWSpinLock_Protect;

/*!
 *  @brief  Structure defining config parameters for the GateHWSpinLock module.
 */
typedef struct GateHWSpinLock_Config {
    GateHWSpinLock_Protect defaultProtection;
    /*!< Default module-wide local context protection level. The level of
     * protection specified here determines which local gate is created per
     * GateHWSpinLock instance for local protection during create. The instance
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
     *   This informs GateHWSpinLock not to pull in the NameServer module. In
     *   this case, all names passed into create and open are ignored.
     */
    UInt32               lockBaseAddr;
    /* Address of lock register base address in HOST OS address space, this is 
     * updated in SysMgr.c */
} GateHWSpinLock_Config;

/*!
 *  @brief  Structure defining config parameters for the GateHWSpinLock
 *          instances.
 */
typedef struct GateHWSpinLock_Params {
    UInt32 lockNum;
    /*!<  Hardware spinlock number */
    String name;
    /*!< If using NameServer, name of this instance. The name (if not NULL) must
     *   be unique among all GateHWSpinLock instances in the entire system.
     */
    GateHWSpinLock_Protect localProtection;
    /*!< Default module-wide local context protection level. The level of
     * protection specified here determines which local gate is created per
     * GateHWSpinLock instance for local protection during create. The instance
     * configuration parameter may be used to override this module setting per
     * instance.  The configuration used here should reflect both the context
     * in which enter and leave are to be called, as well as the maximum level
     * of protection needed locally.
     */
} GateHWSpinLock_Params;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function to get the default configuration for the GateHWSpinLock module. */
Void
GateHWSpinLock_getConfig (GateHWSpinLock_Config * config);

/* Function to setup the GateHWSpinLock module. */
Int
GateHWSpinLock_setup (const GateHWSpinLock_Config * config);

/* Function to destroy the GateHWSpinLock module */
Int
GateHWSpinLock_destroy (Void);

/* Get the default parameters for the GateHWSpinLock instance. */
Void
GateHWSpinLock_Params_init (GateHWSpinLock_Handle   handle,
                            GateHWSpinLock_Params * params);

/* Function to create an instance of GateHWSpinLock */
GateHWSpinLock_Handle
GateHWSpinLock_create (const GateHWSpinLock_Params * params);

/* Function to delete an instance of GateHWSpinLock */
Int
GateHWSpinLock_delete (GateHWSpinLock_Handle * handlePtr);

/* Function to open a previously created GateHWSpinLock instance.  */
Int
GateHWSpinLock_open   (GateHWSpinLock_Handle * handlePtr,
                       GateHWSpinLock_Params * params);

/* Function to close a previously opened instance */
Int
GateHWSpinLock_close  (GateHWSpinLock_Handle * handlePtr);

/* Function to enter the GateHWSpinLock instance */
UInt32
GateHWSpinLock_enter  (GateHWSpinLock_Handle handle);

/* Function to leave the GateHWSpinLock instance */
Void
GateHWSpinLock_leave  (GateHWSpinLock_Handle handle, UInt32 key);


/* =============================================================================
 * Internal functions
 * =============================================================================
 */
/* Returns the GateHWSpinLock kernel object pointer. */
Void *
GateHWSpinLock_getKnlHandle (GateHWSpinLock_Handle handle);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* GATEHWSPINLOCK_H_0xF416 */

