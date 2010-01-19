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
/*
* rcmclient.h
*
* The RCM client module manages the allocation, sending,
* receiving of RCM messages to/ from the RCM server.
*/

#ifndef RCMCLIENT_H_
#define RCMCLIENT_H_

/* Standard headers */
#include <Std.h>
#include <List.h>

/* Utilities headers */
#include <MessageQ.h>

/*!
 *  @def    RCMCLIENT_MODULEID
 *  @brief  Unique module ID.
 */
#define RCMCLIENT_MODULEID              (0xfef2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    RCMCLIENT_STATUSCODEBASE
 *  @brief  Error code base for Rcm Client.
 */
#define RCMCLIENT_STATUSCODEBASE        (RCMCLIENT_MODULEID << 12u)

/*!
 *  @def    RCMCLIENT_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define RCMCLIENT_MAKE_FAILURE(x)       ((Int)  (  0x80000000                  \
                                            + (RCMCLIENT_STATUSCODEBASE  \
                                            + (x))))

/*!
 *  @def    RCMCLIENT_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define RCMCLIENT_MAKE_SUCCESS(x)       (RCMCLIENT_STATUSCODEBASE + (x))

/*
 *  SUCCESS Codes
 *
 */
/* Generic success code for RCMCLIENT module */
#define RCMCLIENT_SOK                   RCMCLIENT_MAKE_SUCCESS(0)

/* The module/ driver is already setup or loaded */
#define RCMCLIENT_SALREADYSETUP         RCMCLIENT_MAKE_SUCCESS(1)

/* The module/ driver is already cleaned up */
#define RCMCLIENT_SALREADYCLEANEDUP     RCMCLIENT_MAKE_SUCCESS(2)

/* The server thread was not shutdown as there are attached clients */
#define RCMCLIENT_SCLIENTSATTACHED      RCMCLIENT_MAKE_SUCCESS(3)

/*
 *  FAILURE Codes
 *
 */
/* This failure code indicates that an operation has timed out. */
#define RCMCLIENT_ETIMEOUT              RCMCLIENT_MAKE_FAILURE(1)

/* This failure code indicates a configuration error */
#define RCMCLIENT_ECONFIG               RCMCLIENT_MAKE_FAILURE(2)

/* This failure code indicates that the RCMCLIENT module has already been
 * initialized.
 */
#define RCMCLIENT_EALREADYINIT          RCMCLIENT_MAKE_FAILURE(3)

/* This failure code indicates that the specified entity was not found */
#define RCMCLIENT_ENOTFOUND             RCMCLIENT_MAKE_FAILURE(4)

/* This failure code indicates that the specified feature is not supported */
#define RCMCLIENT_ENOTSUPPORTED         RCMCLIENT_MAKE_FAILURE(5)

/*
 * This failure code indicates that a provided parameter was outside its valid
 * range.
 */
#define RCMCLIENT_ERANGE                RCMCLIENT_MAKE_FAILURE(6)

/* This failure code indicates that the specified handle is invalid */
#define RCMCLIENT_EHANDLE               RCMCLIENT_MAKE_FAILURE(7)

/* This failure code indicates that an invalid argument was specified */
#define RCMCLIENT_EINVALIDARG           RCMCLIENT_MAKE_FAILURE(8)

/* This failure code indicates a memory related failure */
#define RCMCLIENT_EMEMORY               RCMCLIENT_MAKE_FAILURE(9)

/*
 * This failure code indicates that the RCMCLIENT module
 * has not been initialized
 */
#define RCMCLIENT_EINIT                 RCMCLIENT_MAKE_FAILURE(10)

/* This failure code indicates that a resource was not available.*/
#define RCMCLIENT_ERESOURCE             RCMCLIENT_MAKE_FAILURE(11)

/* This failure code indicates that the specified entity already exists. */
#define RCMCLIENT_EALREADYEXISTS        RCMCLIENT_MAKE_FAILURE(12)

/* This failure code indicates that the specified name is too long. */
#define RCMCLIENT_ENAMELENGTHLIMIT      RCMCLIENT_MAKE_FAILURE(13)

/*
 * This failure code indicates that there was an error in
 * creating/ initializing the object
 */
#define RCMCLIENT_EOBJECT               RCMCLIENT_MAKE_FAILURE(14)

/*
 * This failure code indicates that there was an an issue
 * in object/ resource cleanup
 */
#define RCMCLIENT_ECLEANUP              RCMCLIENT_MAKE_FAILURE(15)

/* This failure code indicates that there was an issue in opening the req. server */
#define RCMCLIENT_ESERVER               RCMCLIENT_MAKE_FAILURE(16)

/* This failure code indicates that there was an issue in sending a message */
#define RCMCLIENT_ESENDMSG              RCMCLIENT_MAKE_FAILURE(17)

/* This failure code indicates that asynch RCM transfers are not enabled */
#define RCMCLIENT_EASYNCENABLED         RCMCLIENT_MAKE_FAILURE(18)

/* This failure code indicates that the symbol was not found */
#define RCMCLIENT_ESYMBOLNOTFOUND       RCMCLIENT_MAKE_FAILURE(19)

/* This failure code indicates that there was an issue in getting a message */
#define RCMCLIENT_EGETMSG               RCMCLIENT_MAKE_FAILURE(20)

/* This failure code indicates that the module is in an invalid state */
#define RCMCLIENT_EINVALIDSTATE         RCMCLIENT_MAKE_FAILURE(21)

/* Generic failure code for RCMCLIENT module */
#define RCMCLIENT_EFAIL                 RCMCLIENT_MAKE_FAILURE(22)

/*
 * RCM message descriptors
 */
#define RcmClient_Desc_RCM_MSG          0x1 /*  RcmClient execution message */
#define RcmClient_Desc_DPC              0x2 /*  Deferred Procedure Call */
#define RcmClient_Desc_SYM_ADD          0x3 /*  Symbol add message */
#define RcmClient_Desc_SYM_IDX          0x4 /*  Query symbox index */
#define RcmClient_Desc_SHUTDOWN         0x5 /*  RcmServer shutdown message */
#define RcmClient_Desc_CONNECT          0x6 /*  RcmClient connected message */
#define RcmClient_Desc_RCM_NO_REPLY     0x7 /*  RcmClient No Reply */

/*
 * RCM client default heap ID
 */
#define RCMCLIENT_DEFAULT_HEAPID        0xFFFF
#define RCMCLIENT_INVALID_MESSAGE_ID    0xFFFF

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining config parameters for the RcmClient module.
 */
typedef struct RcmClient_Config_tag {
    UInt32 maxNameLen;
    /* Maximum length of name */
    UInt16 defaultHeapIdArrayLength;
    /* No of entries in the default heapID array */
    UInt16 defaultHeapBlockSize;
    /* Blocksize of the entries in the default heapID array */
} RcmClient_Config;

/*
 * Instance config-params object.
 */
typedef struct RcmClient_Params_tag {
    UInt16 heapId; /* heap ID for msg alloc */
    Bool   callbackNotification; /* enable/ disable asynchronous exec */
} RcmClient_Params;

/*
 * RCM message structure.
 */
typedef struct RcmClient_Message_tag {
    UInt32   fxnIdx;    /*  remote function index */
    Int32    result;    /*  function return value */
    UInt32   dataSize;  /*  read-only size of data block (in chars) */
    UInt32   data[1];   /*  data buffer of dataSize chars */
} RcmClient_Message;

/*
 * RCM remote function pointer.
 */
typedef Int32 (*RcmClient_RemoteFuncPtr)(UInt32, UInt32 *);

/*
 * RCM callback function pointer
 */
typedef Void (*RcmClient_CallbackFuncPtr)(RcmClient_Message *, Ptr);

/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */

/*!
 *  @brief  Handle for the RcmClient.
 */
typedef struct RcmClient_Object_tag *RcmClient_Handle;

/* =============================================================================
 *  APIs
 * =============================================================================
 */

/* Function to get default configuration for the RcmClient module */
Int RcmClient_getConfig (RcmClient_Config *cfgParams);

/* Function to setup the RCM Client module */
Int RcmClient_setup (const RcmClient_Config *config);

/* Function to destroy the RCM Client module */
Int RcmClient_destroy (void);

/* Function to create a RCM client instance */
Int RcmClient_create (String                     server,
                      const RcmClient_Params *   params,
                      RcmClient_Handle *         rcmclientHandle);

/* Function to delete RCM Client instance */
Int RcmClient_delete (RcmClient_Handle * handlePtr);

/* Initialize this config-params structure with supplier-specified defaults */
Int RcmClient_Params_init (RcmClient_Handle      handle,
                           RcmClient_Params *    params);

/* Function adds symbol to server, return the function index */
Int RcmClient_addSymbol (RcmClient_Handle        handle,
                         String                  funcName,
                         RcmClient_RemoteFuncPtr address,
                         UInt32 *                fxn_idx);

/* Function returns size (in bytes) of RCM header. */
Int RcmClient_getHeaderSize (Void);

/*
 * Function allocates memory for RCM message on heap,
 * populates MessageQ and RCM message.
 */
RcmClient_Message *RcmClient_alloc (RcmClient_Handle handle, UInt32 dataSize);

/* Function requests RCM server to execute remote function */
Int RcmClient_exec (RcmClient_Handle    handle,
                    RcmClient_Message * rcmMsg);

/*Function requests RCM  server to execute remote function, it is asynchronous*/
Int RcmClient_execAsync (RcmClient_Handle           handle,
                         RcmClient_Message *        rcmMsg,
                         RcmClient_CallbackFuncPtr  callback,
                         Ptr                        appData);

/*
 * Function requests RCM server to execute remote function,
 * does not wait for completion of remote function for reply
 */
Int RcmClient_execDpc (RcmClient_Handle     handle,
                       RcmClient_Message *  rcmMsg);

/*
 * Function requests RCM server to execute remote function,
 * provides a msgId to wait on later
 */
Int RcmClient_execNoWait (RcmClient_Handle      handle,
                          RcmClient_Message *   rcmMsg,
                          UInt16 *              msgId);
/*
 * Function requests RCM server to execute remote function,
 * without waiting for the reply.
 */
Int RcmClient_execNoReply (RcmClient_Handle      handle,
                           RcmClient_Message *   rcmMsg);


/* Function frees the RCM message and allocated memory  */
Void RcmClient_free (RcmClient_Handle handle, RcmClient_Message *rcmMsg);

/* Function gets symbol index */
Int RcmClient_getSymbolIndex (RcmClient_Handle  handle,
                              String            funcName,
                              UInt32 *          fxnIdx);

/* Function removes symbol (remote function) from registry */
Int RcmClient_removeSymbol (RcmClient_Handle handle, String funcName);

/*
 * Function waits till invoked remote function completes
 * return message will contain result and context
 */
Int RcmClient_waitUntilDone (RcmClient_Handle       handle,
                             UInt16                 msgId,
                             RcmClient_Message *    rcmMsg);

#endif /* RCMCLIENT_H_ */
