/*
 *  Syslink-IPC for TI OMAP Processors
 *
 *  Copyright (c) 2008-2010, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <ti/ipc/MessageQ.h>

/*
 *  @def    RCMSERVER_MODULEID
 *   Unique module ID.
 */
#define RCMSERVER_MODULEID              (0xeee2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*
 *  Success return code
 */
#define RcmServer_S_SUCCESS (0)

/*
 *  Module already setup
 */
#define RcmServer_S_ALREADYSETUP (1)

/*
 *  Module already cleaned up
 */
#define RcmServer_S_ALREADYCLEANEDUP (1)

/*
 *  General failure return code
 */
#define RcmServer_E_FAIL (-1)

/*
 *  Invalid function index
 *
 *  An RcmClient_Message was received which contained a function
 *  index value that was not found in the server's function table.
 */
#define RcmServer_E_INVALIDFXNIDX (-2)

/*
 *  An unknown error has been detected from the IPC layer
 *
 *  Check the error log for additional information.
 */
#define RcmServer_E_IPCERR (-3)

/*
 *  The client message queue could not be created
 *
 *  Each RcmClient instance must create its own message queue for
 *  receiving return messages from the RcmServer. The creation of
 *  this message queue failed, thus failing the RcmClient instance
 *  creation.
 */
#define RcmServer_E_MSGQCREATEFAILED (-4)

/*
 *  The given symbol was not found in the server's symbol table
 *
 *  This error could occur if the symbol spelling is incorrect or
 *  if the RcmServer is still loading its symbol table.
 */
#define RcmServer_E_SYMBOLNOTFOUND (-5)

/*
 *  The given symbols is in the static table, it cannot be removed
 *
 *  All symbols installed at instance create time are added to the
 *  static symbol table. They cannot be removed. The statis symbol
 *  table must remain intact for the lifespan of the server instance.
 */
#define RcmServer_E_SYMBOLSTATIC (-6)

/*
 *  The server's symbol table is full
 *
 *  The symbol table is full. You must remove some symbols before
 *  any new symbols can be added.
 */
#define RcmServer_E_SYMBOLTABLEFULL (-7)

/*
 *  The was insufficient memory left on the heap
 */
#define RcmServer_E_NOMEMORY (-8)

/*
 *  Invalid argument
 */
#define RcmServer_E_INVALIDARG (-9)

/*
 *  Module not initialized
 */
#define RcmServer_E_INVALIDSTATE (-10)


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

/* server status codes must be 0 - 15, it has to fit in a 4-bit field */
#define RcmServer_Status_SUCCESS ((UInt16)(0)) // success
#define RcmServer_Status_INVALID_FXN ((UInt16)(1)) // invalid function index
#define RcmServer_Status_SYMBOL_NOT_FOUND ((UInt16)(2)) // symbol not found
#define RcmServer_Status_INVALID_MSG_TYPE ((UInt16)(3)) // invalid message type
#define RcmServer_Status_MSG_FXN_ERR ((UInt16)(4)) // message function error
#define RcmServer_Status_ERROR ((UInt16)(5)) // general failure

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*
 *   Structure defining config parameters for the RcmClient module.
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

/*
 *   Remote function type
 *
 *  All functions executed by the RcmServer must be of this
 *  type. Typically, these functions are simply wrappers to the vendor
 *  function. The server invokes this remote function by passing in
 *  the RcmClient_Message.dataSize field and the address of the
 *  RcmClient_Message.data array.
 *
 *  RcmServer_MsgFxn fxn = ...;
 *  RcmClient_Message *msg = ...;
 *  msg->result = (*fxn)(msg->dataSize, msg->data);
 */
typedef Int32 (*RcmServer_MsgFxn)(UInt32, UInt32 *);


/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */

/*!
 *   Handle for the RcmServer.
 */
typedef struct RcmServer_Object_tag *RcmServer_Handle;

/* =============================================================================
 *  APIs
 * =============================================================================
 */

/* Function to setup RCM server */
Void RcmServer_init (Void);

/* Function to clean up RCM server */
Void RcmServer_exit (Void);

/* Function will create a RCM Server instance */
Int RcmServer_create (String                     name,
                      RcmServer_Params *         params,
                      RcmServer_Handle *         rcmserverHandle);

/* Function will delete a RCM Server instance */
Int RcmServer_delete(RcmServer_Handle * handlePtr);

/* Initialize this config-params structure with supplier-specified defaults */
Int RcmServer_Params_init (RcmServer_Params *    params);

/* Function adds symbol to server, return the function index */
Int RcmServer_addSymbol (RcmServer_Handle        handle,
                         String                  funcName,
                         RcmServer_MsgFxn        address,
                         UInt32 *                fxnIdx);

/* Function removes symbol from server */
Int RcmServer_removeSymbol (RcmServer_Handle handle, String funcName);

/* Function starts RCM server thread by posting sync */
Void RcmServer_start (RcmServer_Handle handle);

#endif /* RCMSERVER_H_ */
