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
 *  @file   MessageQTransportShm.h
 *
 *  @brief      MessageQ shared memory based physical transport for
 *              communication with the remote processor.
 *
 *              This file contains the declarations of types and APIs as part
 *              of interface of the MessageQ shared memory transport.
 **  ============================================================================
 */


#ifndef MESSAGEQTRANSPORTSHM_H_0x0a7a
#define MESSAGEQTRANSPORTSHM_H_0x0a7a


/* Standard headers */
#include <List.h>
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    MESSAGEQTRANSPORTSHM_MODULEID
 *  @brief  Unique module ID.
 */
#define MESSAGEQTRANSPORTSHM_MODULEID               (0x0a7a)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    MESSAGEQTRANSPORTSHM_STATUSCODEBASE
 *  @brief  Error code base for MessageQ.
 */
#define MESSAGEQTRANSPORTSHM_STATUSCODEBASE                              \
                                    (MESSAGEQTRANSPORTSHM_MODULEID << 12u)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define MESSAGEQTRANSPORTSHM_MAKE_FAILURE(x)  ((Int)  (  0x80000000            \
                                       + (MESSAGEQTRANSPORTSHM_STATUSCODEBASE  \
                                       + (x))))

/*!
 *  @def    MESSAGEQTRANSPORTSHM_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define MESSAGEQTRANSPORTSHM_MAKE_SUCCESS(x)                              \
                                    (MESSAGEQTRANSPORTSHM_STATUSCODEBASE + (x))

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define MESSAGEQTRANSPORTSHM_E_INVALIDARG   MESSAGEQTRANSPORTSHM_MAKE_FAILURE(1)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_INVALIDSIZE
 *  @brief  Invalid shared address size
 */
#define MESSAGEQTRANSPORTSHM_E_INVALIDSIZE  MESSAGEQTRANSPORTSHM_MAKE_FAILURE(2)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define MESSAGEQTRANSPORTSHM_E_INVALIDSTATE MESSAGEQTRANSPORTSHM_MAKE_FAILURE(3)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_BADVERSION
 *  @brief  Versions don't match
 */
#define MESSAGEQTRANSPORTSHM_E_BADVERSION   MESSAGEQTRANSPORTSHM_MAKE_FAILURE(4)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_FAIL
 *  @brief  General Failure
*/
#define MESSAGEQTRANSPORTSHM_E_FAIL         MESSAGEQTRANSPORTSHM_MAKE_FAILURE(5)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_MEMORY
 *  @brief  Memory allocation failed
 */
#define MESSAGEQTRANSPORTSHM_E_MEMORY       MESSAGEQTRANSPORTSHM_MAKE_FAILURE(6)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define MESSAGEQTRANSPORTSHM_E_OSFAILURE    MESSAGEQTRANSPORTSHM_MAKE_FAILURE(7)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_HANDLE
 *  @brief  Invalid handle specified.
 */
#define MESSAGEQTRANSPORTSHM_E_HANDLE       MESSAGEQTRANSPORTSHM_MAKE_FAILURE(8)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_E_NOTSUPPORTED
 *  @brief  The specified operation is not supported.
 */
#define MESSAGEQTRANSPORTSHM_E_NOTSUPPORTED MESSAGEQTRANSPORTSHM_MAKE_FAILURE(9)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_SUCCESS
 *  @brief  Operation successful.
 */
#define MESSAGEQTRANSPORTSHM_SUCCESS        MESSAGEQTRANSPORTSHM_MAKE_SUCCESS(0)

/*!
 *  @def    MESSAGEQTRANSPORTSHM_S_ALREADYSETUP
 *  @brief  The MESSAGETRANSPORTSHM module has
 *          already been setup in this process.
 */
#define MESSAGEQTRANSPORTSHM_S_ALREADYSETUP MESSAGEQTRANSPORTSHM_MAKE_SUCCESS(1)


/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */
/*! @brief Forward declaration of structure defining object for the
 *         MessageQTransportShm
 */
typedef struct MessageQTransportShm_Object_tag MessageQTransportShm_Object;

/*!
 *  @brief  Handle for the MessageQTransportShm
 */
typedef struct MessageQTransportShm_Object * MessageQTransportShm_Handle;


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining the reason for error function being called
 */
typedef enum  MessageQTransportShm_Reason_tag {
    MessageQTransportShm_Reason_FAILEDPUT,
    /*!< Failed to send the message. */
    MessageQTransportShm_Reason_INTERNALERR,
    /*!< An internal error occurred in the transport */
    MessageQTransportShm_Reason_PHYSICALERR,
    /*!<  An error occurred in the physical link in the transport */
    MessageQTransportShm_Reason_FAILEDALLOC
    /*!<  Failed to allocate a message. */
} MessageQTransportShm_Reason;

/*!
 *  @brief  Typedef for transport error callback function.
 *
 *  First parameter: Why the error function is being called.
 *
 *  Second parameter: Handle of transport that had the error. NULL denotes
 *  that it is a system error, not a specific transport.
 *
 *  Third parameter: Pointer to the message. This is only valid for
 *  #MessageQTransportShm_Reason_FAILEDPUT.
 *
 *  Fourth parameter: Transport specific information. Refer to individual
 *  transports for more details.
 */
typedef Void (*MessageQTransportShm_ErrFxn) (MessageQTransportShm_Reason reason,
                                             MessageQTransportShm_Handle handle,
                                             Ptr                         msg,
                                             UArg                        info);

/*!
 *  @brief  Module configuration structure.
 */
typedef struct MessageQTransportShm_Config {
    MessageQTransportShm_ErrFxn errFxn;
    /*!< Asynchronous error function for the transport module */
} MessageQTransportShm_Config;

/*!
 *  @brief  Structure defining config parameters for the MessageQ transport
 *  instances.
 */
typedef struct MessageQTransportShm_Params_tag {
    UInt            priority;
    /*!<  Priority of messages supported by this transport */
    Gate_Handle     gate;
    /*!< Gate used for critical region management of the shared memory */
    Ptr             sharedAddr;
    /*!<  Address of the shared memory. The creator must supply the shared
     *    memory that this will use for maintain shared state information.
     */
    UInt32          sharedAddrSize;
    /*!<  Size of shared region provided. */
    UInt32          notifyEventNo;
    /*!<  Notify event number to be used by the transport */
    Ptr             notifyDriver;
    /*!<  Notify driver to be used by the transport */
} MessageQTransportShm_Params;

/*!
 *  @brief  Structure defining Transport status values
 */
typedef enum  MessageQTransportShm_Status_tag {
    MessageQTransportShm_Status_INIT,
    /*!< MessageQ transport Shm instance has not not completed
     * initialization.
     */
    MessageQTransportShm_Status_UP,
    /*!< MessageQ transport Shm instance is up and functional. */
    MessageQTransportShm_Status_DOWN,
    /*!<  MessageQ transport Shm instance is down and not functional. */
    MessageQTransportShm_Status_RESETTING
    /*!<  MessageQ transport Shm instance was up at one point and is in
     * process of resetting.
     */
} MessageQTransportShm_Status;


/* =============================================================================
 *  APIs called by applications
 * =============================================================================
 */
/* Function to get the default configuration for the MessageQTransportShm
 * module.
 */
Void MessageQTransportShm_getConfig (MessageQTransportShm_Config * cfg);

/* Function to setup the MessageQTransportShm module. */
Int MessageQTransportShm_setup (const MessageQTransportShm_Config * cfg);

/* Function to destroy the MessageQTransportShm module. */
Int MessageQTransportShm_destroy (Void);

/* Get the default parameters for the NotifyShmDriver. */
Void MessageQTransportShm_Params_init (MessageQTransportShm_Handle      handle,
                                       MessageQTransportShm_Params    * params);

/* Create an instance of the MessageQTransportShm. */
MessageQTransportShm_Handle MessageQTransportShm_create
                             (      UInt16                         procId,
                              const MessageQTransportShm_Params  * params);

/* Delete an instance of the MessageQTransportShm. */
Int MessageQTransportShm_delete (MessageQTransportShm_Handle * handlePtr);

/* Get the shared memory requirements for the MessageQTransportShm. */
UInt32
MessageQTransportShm_sharedMemReq (const MessageQTransportShm_Params * params);

/* Set the asynchronous error function for the transport module */
Void MessageQTransportShm_setErrFxn (MessageQTransportShm_ErrFxn errFxn);


/* =============================================================================
 *  APIs called internally by MessageQ module.
 * =============================================================================
 */
/* Put msg to remote list */
Int  MessageQTransportShm_put (MessageQTransportShm_Handle   handle,
                               Ptr                           msg);

/* Control Function */
Int MessageQTransportShm_control (MessageQTransportShm_Handle handle,
                                  UInt                        cmd,
                                  UArg                        cmdArg);
/* Get current status of the MessageQTransportShm */
MessageQTransportShm_Status
MessageQTransportShm_getStatus (MessageQTransportShm_Handle obj);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MESSAGEQTRANSPORTSHM_H_0x0a7a */

