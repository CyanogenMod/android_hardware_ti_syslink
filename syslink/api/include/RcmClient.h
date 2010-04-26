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
#include <ti/ipc/MessageQ.h>

/*!
 *  @def    RCMCLIENT_MODULEID
 *   Unique module ID.
 */
#define RCMCLIENT_MODULEID              (0xfef2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*
 *  Success return code
 */
#define RcmClient_S_SUCCESS (0)

/*
 *  Success, already set up
 */
#define RcmClient_S_ALREADYSETUP (1)

/*
 *  Success, already cleaned up
 */
#define RcmClient_S_ALREADYCLEANEDUP (2)

/*
 *  General failure return code
 */
#define RcmClient_E_FAIL (-1)

/*
 *  The client has not been configured for asynchronous notification
 *
 *  In order to use the RcmClient_execAsync() function, the RcmClient
 *  must be configured with callbackNotification set to true in the
 *  instance create parameters.
 */
#define RcmClient_E_EXECASYNCNOTENABLED (-2)

/*
 *  The client was unable to send the command message to the server
 *
 *  An IPC transport error occurred. The message was never sent to the server.
 */
#define RcmClient_E_EXECFAILED (-3)

/*
 *  A heap id must be provided in the create params
 *
 *  When an RcmClient instance is created, a heap id must be given
 *  in the create params. This heap id must be registered with MessageQ
 *  before calling RcmClient_create().
 */
#define RcmClient_E_INVALIDHEAPID (-4)

/*
 *  Invalid function index
 *
 *  An RcmClient_Message was sent to the server which contained a
 *  function index value (in the fxnIdx field) that was not found
 *  in the server's function table.
 */
#define RcmClient_E_INVALIDFXNIDX (-5)

/*
 *  Message function error
 *
 *  There was an error encountered in either the message function or
 *  the library function invoked by the message function. The semantics
 *  of the error code are implementation dependent.
 */
#define RcmClient_E_MSGFXNERR (-6)

/*
 *  An unknown error has been detected from the IPC layer
 *
 *  Check the error log for additional information.
 */
#define RcmClient_E_IPCERR (-7)

/*
 *  Failed to create the list object
 */
#define RcmClient_E_LISTCREATEFAILED (-8)

/*
 *  The expected reply message from the server was lost
 *
 *  A command message was sent to the RcmServer but the reply
 *  message was not received. This is an internal error.
 */
#define RcmClient_E_LOSTMSG (-9)

/*
 *  Insufficient memory to allocate a message
 *
 *  The message heap cannot allocate a buffer of the requested size.
 *  The reported size it the requested data size and the underlying
 *  message header size.
 */
#define RcmClient_E_MSGALLOCFAILED (-10)

/*
 *  The client message queue could not be created
 *
 *  Each RcmClient instance must create its own message queue for
 *  receiving return messages from the RcmServer. The creation of
 *  this message queue failed, thus failing the RcmClient instance
 *  creation.
 */
#define RcmClient_E_MSGQCREATEFAILED (-11)

/*
 *  The server message queue could not be opened
 *
 *  Each RcmClient instance must open the server's message queue.
 *  This error is raised when an internal error occurred while trying
 *  to open the server's message queue.
 */
#define RcmClient_E_MSGQOPENFAILED (-12)

/*
 *  The server returned an unknown error code
 *
 *  The server encountered an error with the given message but
 *  the error code is not recognized by the client.
 */
#define RcmClient_E_SERVERERROR (-13)

/*
 *  The server specified in the create params was not found
 *
 *  When creating an RcmClient instance, the specified server could not
 *  be found. This could occur if the server name is incorrect, or
 *  if the RcmClient instance is created before the RcmServer. In such an
 *  instance, the client can retry when the RcmServer is expected to
 *  have been created.
 */
#define RcmClient_E_SERVERNOTFOUND (-14)

/*
 *  The given symbol was not found in the server symbol table
 *
 *  This error could occur if the symbol spelling is incorrect or
 *  if the RcmServer is still loading its symbol table.
 */
#define RcmClient_E_SYMBOLNOTFOUND (-15)

/*
 *  There is insufficient memory left in the heap
 */
#define RcmClient_E_NOMEMORY (-16)

/*
 *  Invalid argument
 */
#define RcmClient_E_INVALIDARG (-17)

/*
 *  Invalid module state
 */
#define RcmClient_E_INVALIDSTATE (-18)

/*
 *  Function not supported
 */
#define RcmClient_E_NOTSUPPORTED (-19)


/* =============================================================================
 *  constants and types
 * =============================================================================
 */
/*
 *  Invalid function index
 */
#define RcmClient_INVALIDFXNIDX ((UInt32)(0xFFFFFFFF))

/*
 *  Invalid heap id
 */
#define RcmClient_INVALIDHEAPID ((UInt16)(0xFFFF))

/*
 *  Invalid message id
 */
#define RcmClient_INVALIDMSGID ((UInt16)(0))


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

#define RcmClient_Desc_TYPE_MASK  0x0F00    /* field mask */
#define RcmClient_Desc_TYPE_SHIFT 8         /* field shift width */

/* server status codes must be 0 - 15, it has to fit in a 4-bit field */
#define RcmServer_Status_SUCCESS ((UInt16)(0)) /* success */
#define RcmServer_Status_INVALID_FXN ((UInt16)(1)) /* invalid function index */
#define RcmServer_Status_SYMBOL_NOT_FOUND ((UInt16)(2)) /* symbol not found */
#define RcmServer_Status_INVALID_MSG_TYPE ((UInt16)(3)) /* invalid message type */
#define RcmServer_Status_MSG_FXN_ERR ((UInt16)(4)) /* message function error */
#define RcmServer_Status_ERROR ((UInt16)(5)) /* general failure */

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
 *   Structure defining config parameters for the RcmClient module.
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
typedef Int32 (*RcmClient_RemoteFuncPtr)(RcmClient_Message *, Ptr);

/*
 * RCM callback function pointer
 */
typedef Void (*RcmClient_CallbackFxn)(RcmClient_Message *, Ptr);

/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */

/*!
 *   Handle for the RcmClient.
 */
typedef struct RcmClient_Object_tag *RcmClient_Handle;

/* =============================================================================
 *  APIs
 * =============================================================================
 */

/* Function to setup RCM client */
Void RcmClient_init (Void);

/* Function to clean up RCM client */
Void RcmClient_exit (Void);

/* Function to create a RCM client instance */
Int RcmClient_create (String                     server,
                      RcmClient_Params *         params,
                      RcmClient_Handle *         handle);

/* Function to delete RCM Client instance */
Int RcmClient_delete (RcmClient_Handle * handlePtr);

/* Initialize this config-params structure with supplier-specified defaults */
Int RcmClient_Params_init (RcmClient_Params *    params);

/* Function adds symbol to server, return the function index */
Int RcmClient_addSymbol (RcmClient_Handle        handle,
                         String                  name,
                         RcmClient_RemoteFuncPtr addr,
                         UInt32 *                index);

/* Function returns size (in bytes) of RCM header. */
Int RcmClient_getHeaderSize (Void);

/*
 * Function allocates memory for RCM message on heap,
 * populates MessageQ and RCM message.
 */
Int RcmClient_alloc (RcmClient_Handle handle,
                     UInt32 dataSize,
                     RcmClient_Message **message);

/* Function requests RCM server to execute remote function */
Int RcmClient_exec (RcmClient_Handle    handle,
                    RcmClient_Message * cmdMsg,
                    RcmClient_Message **returnMsg);

/*Function requests RCM  server to execute remote function, it is asynchronous*/
Int RcmClient_execAsync (RcmClient_Handle           handle,
                         RcmClient_Message *        cmdMsg,
                         RcmClient_CallbackFxn      callback,
                         Ptr                        appData);

/*
 * Function requests RCM server to execute remote function,
 * does not wait for completion of remote function for reply
 */
Int RcmClient_execDpc (RcmClient_Handle     handle,
                       RcmClient_Message *  cmdMsg,
                       RcmClient_Message ** returnMsg);

/*
 * Function requests RCM server to execute remote function,
 * provides a msgId to wait on later
 */
Int RcmClient_execNoWait (RcmClient_Handle      handle,
                          RcmClient_Message *   cmdMsg,
                          UInt16 *              msgId);
/*
 * Function requests RCM server to execute remote function,
 * without waiting for the reply.
 */
Int RcmClient_execNoReply (RcmClient_Handle      handle,
                           RcmClient_Message *   cmdMsg);


/* Function frees the RCM message and allocated memory  */
Int RcmClient_free (RcmClient_Handle handle, RcmClient_Message *msg);

/* Function gets symbol index */
Int RcmClient_getSymbolIndex (RcmClient_Handle  handle,
                              String            name,
                              UInt32 *          index);

/* Function removes symbol (remote function) from registry */
Int RcmClient_removeSymbol (RcmClient_Handle handle, String name);

/*
 * Function waits till invoked remote function completes
 * return message will contain result and context
 */
Int RcmClient_waitUntilDone (RcmClient_Handle       handle,
                             UInt16                 msgId,
                             RcmClient_Message **   returnMsg);

#endif /* RCMCLIENT_H_ */
