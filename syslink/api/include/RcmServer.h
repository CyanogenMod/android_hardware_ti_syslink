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
* rcmserver.h
*
* The RCM server module receives RCM messages from the RCM client,
* executes remote function and sends replies to the RCM client.
*/

#ifndef RCMSERVER_H_
#define RCMSERVER_H_

/* Standard headers */
#include <Std.h>
#include <List.h>

/* Utilities headers */
#include <MessageQ.h>

/*!
 *  @def    RCMSERVER_MODULEID
 *  @brief  Unique module ID.
 */
#define RCMSERVER_MODULEID              (0xeee2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    RCMSERVER_STATUSCODEBASE
 *  @brief  Error code base for RCM Server.
 */
#define RCMSERVER_STATUSCODEBASE        (RCMSERVER_MODULEID << 12u)

/*!
 *  @def    RCMSERVER_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define RCMSERVER_MAKE_FAILURE(x)       ((Int) ( 0x80000000                    \
                                             + (RCMSERVER_STATUSCODEBASE       \
                                             + (x))))

/*!
 *  @def    RCMSERVER_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define RCMSERVER_MAKE_SUCCESS(x)       (RCMSERVER_STATUSCODEBASE + (x))


/*
 *  SUCCESS Codes
 *
 */
/* Generic success code for RCMSERVER module */
#define RCMSERVER_SOK               RCMSERVER_MAKE_SUCCESS(0)

/* The module/ driver is already setup or loaded */
#define RCMSERVER_SALREADYSETUP     RCMSERVER_MAKE_SUCCESS(1)

/* The module/ driver is already cleaned up */
#define RCMSERVER_SALREADYCLEANEDUP RCMSERVER_MAKE_SUCCESS(2)

/*
 *  FAILURE Codes
 *
 */
/* This failure code indicates that an operation has timed out. */
#define RCMSERVER_ETIMEOUT          RCMSERVER_MAKE_FAILURE(1)

/* This failure code indicates a configuration error */
#define RCMSERVER_ECONFIG           RCMSERVER_MAKE_FAILURE(2)

/* This failure code indicates that the RCMSERVER module has already been
 * initialized.
 */
#define RCMSERVER_EALREADYINIT      RCMSERVER_MAKE_FAILURE(3)

/* This failure code indicates that the specified entity was not found */
#define RCMSERVER_ENOTFOUND         RCMSERVER_MAKE_FAILURE(4)

/* This failure code indicates that the specified feature is not supported */
#define RCMSERVER_ENOTSUPPORTED     RCMSERVER_MAKE_FAILURE(5)

/* This failure code indicates that a provided parameter was outside its valid
 * range.
 */
#define RCMSERVER_ERANGE            RCMSERVER_MAKE_FAILURE(6)

/* This failure code indicates that the specified handle is invalid */
#define RCMSERVER_EHANDLE           RCMSERVER_MAKE_FAILURE(7)

/* This failure code indicates that an invalid argument was specified */
#define RCMSERVER_EINVALIDARG       RCMSERVER_MAKE_FAILURE(8)

/* This failure code indicates a memory related failure */
#define RCMSERVER_EMEMORY           RCMSERVER_MAKE_FAILURE(9)

/*
 *  This failure code indicates that the RCMSERVER module
 *  has not been initialized
 */
#define RCMSERVER_EINIT             RCMSERVER_MAKE_FAILURE(10)

/* This failure code indicates that a resource was not available.*/
#define RCMSERVER_ERESOURCE         RCMSERVER_MAKE_FAILURE(11)

/* This failure code indicates that the specified entity already exists. */
#define RCMSERVER_EALREADYEXISTS    RCMSERVER_MAKE_FAILURE(12)

/* This failure code indicates that the specified name is too long. */
#define RCMSERVER_ENAMELENGTHLIMIT  RCMSERVER_MAKE_FAILURE(13)

/*
 * This failure code indicates that there was an error in
 * creating/ initializing the object
 */
#define RCMSERVER_EOBJECT           RCMSERVER_MAKE_FAILURE(14)

/*
 * This failure code indicates that there was an an issue in
 * object/ resource cleanup
 */
#define RCMSERVER_ECLEANUP          RCMSERVER_MAKE_FAILURE(15)

/*
 * This failure code indicates that there was no space
 * in the fxn table to store the symbol
 */
#define RCMSERVER_ENOFREESLOT       RCMSERVER_MAKE_FAILURE(16)

/* This failure code indicates that the symbol was not found */
#define RCMSERVER_ESYMBOLNOTFOUND   RCMSERVER_MAKE_FAILURE(17)

/* This failure code indicates that there was an issue in sending a message */
#define RCMSERVER_ESENDMSG          RCMSERVER_MAKE_FAILURE(18)

/* This failure code indicates that the module is in an invalid state */
#define RCMSERVER_EINVALIDSTATE     RCMSERVER_MAKE_FAILURE(19)

/* This failure code indicates that there was an issue in getting a message */
#define RCMSERVER_EGETMSG           RCMSERVER_MAKE_FAILURE(20)

/* This failure code indicates that there was an issue freeing the message */
#define RCMSERVER_EFREEMSG          RCMSERVER_MAKE_FAILURE(21)

/*FIXME have a different value for EFAIL */
/* Generic failure code for RCMSERVER module */
#define RCMSERVER_EFAIL             RCMSERVER_MAKE_FAILURE(20)

/*
 * RCM message descriptors
 */
#define RcmClient_Desc_RCM_MSG      0x1     /* RCMSERVER execution message */
#define RcmClient_Desc_DPC          0x2     /* Deferred Procedure Call */
#define RcmClient_Desc_SYM_ADD      0x3     /* Symbol add message */
#define RcmClient_Desc_SYM_IDX      0x4     /* Query symbox index */
#define RcmClient_Desc_SHUTDOWN     0x5     /* RcmServer shutdown message */
#define RcmClient_Desc_CONNECT      0x6     /* RcmClient connected message */
#define RcmClient_Desc_RCM_NO_REPLY 0x7     /* RcmClient No Reply */

#define RCMSERVER_HIGH_PRIORITY     0xFF
#define RCMSERVER_REGULAR_PRIORITY  0x01

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining config parameters for the RcmClient module.
 */
typedef struct RcmServer_Config_tag {
    UInt32 maxNameLen; /* Maximum length of name */
    UInt16 maxTables;  /* No of dynamic tables for remote function storage */
} RcmServer_Config;

/*
 * Instance config-params object.
 */
typedef struct RcmServer_Params_tag {
    Int priority;       /* Priority of RCM server */
} RcmServer_Params;

typedef struct RcmServer_Message_tag {
    UInt32    fxnIdx;   /*  remote function index */
    UInt32    result;   /*  function return value */
    UInt32    dataSize; /*  read-only size of data block (in chars) */
    UInt32    data[1];  /*  data buffer of dataSize chars */
} RcmServer_Message;

typedef Int32 (*RcmServer_RemoteFuncPtr)(UInt32, UInt32 *);

/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */

/*!
 *  @brief  Handle for the RcmServer.
 */
typedef struct RcmServer_Object_tag *RcmServer_Handle;

/* =============================================================================
 *  APIs
 * =============================================================================
 */

/* Function to get default configuration for the RcmServer module */
Int RcmServer_getConfig (RcmServer_Config * cfgParams);

/* Function will create/ start the RCM Server module */
Int RcmServer_setup (const RcmServer_Config * config);

/* Function will destroy the RCM Server module */
Int RcmServer_destroy(void);

/* Function will create a RCM Server instance */
Int RcmServer_create (String                     name,
                      const RcmServer_Params *   params,
                      RcmServer_Handle *         rcmserverHandle);

/* Function will delete a RCM Server instance */
Int RcmServer_delete(RcmServer_Handle * handlePtr);

/* Initialize this config-params structure with supplier-specified defaults */
Int RcmServer_Params_init (RcmServer_Handle      handle,
                           RcmServer_Params *    params);

/* Function adds symbol to server, return the function index */
Int RcmServer_addSymbol (RcmServer_Handle        handle,
                         String                  funcName,
                         RcmServer_RemoteFuncPtr address,
                         UInt32 *                fxnIdx);

/* Function removes symbol from server */
Int RcmServer_removeSymbol (RcmServer_Handle handle, String funcName);

/* Function starts RCM server thread by posting sync */
Int RcmServer_start (RcmServer_Handle handle);

#endif /* RCMSERVER_H_ */
