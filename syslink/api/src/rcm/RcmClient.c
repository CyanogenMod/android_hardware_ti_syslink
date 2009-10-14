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
 * RcmClient.c
 *
 * The RCM client module manages the allocation, sending,
 * receiving of RCM messages to/ from the RCM server.
 *
 */

/*
 *  ======== RcmClient.c ========
 *  Notes:
 *  NA
 */

/* Standard headers */
#include <host_os.h>
#include <pthread.h>

/* OSAL, Utility and IPC headers */
#include <Std.h>
#include <Trace.h>
#include <MessageQ.h>
#include <GateMutex.h>
#include <OsalSemaphore.h>
#include <Memory.h>
#include <List.h>

/* Module level headers */
#include <RcmClient.h>

/* =============================================================================
 * RCM client defines
 * =============================================================================
 */
#define RCMCLIENT_INVALID_FUNCTION_INDEX 0xFFFFFFFF
#define RCMCLIENT_DESC_TYPE_SHIFT 8 /* field shift width */
#define RCMCLIENT_HEAPID_ARRAY_LENGTH 4
#define RCMCLIENT_HEAPID_ARRAY_BLOCKSIZE 256
#define MAX_NAME_LEN 32
#define WAIT_NONE 0x0

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/*
 * RCM Client packet structure
 */
typedef struct RcmClient_Packet_tag{
    MessageQ_MsgHeader msgqHeader; /* MessageQ header */
    UInt16 desc; /* protocol version, descriptor, status */
    UInt16 msgId; /* message id */
    RcmClient_Message message; /* client message body (4 words) */
} RcmClient_Packet;

/*
 * recipient list elem structure
 */
typedef struct Recipient_tag {
    List_Elem elem;
    UInt16 msgId;
    RcmClient_Message *msg;
    OsalSemaphore_Handle event;
} Recipient;

/*
 * RCM Client instance object structure
 */
typedef struct RcmClient_Object_tag {
    MessageQ_Handle msgQ;    /* inbound message queue */
    MessageQ_QueueId serverMsgQ; /* server message queue id*/
    UInt16  heapId; /* heap used for message allocation */
    Bool cbNotify; /* callback notification */
    UInt16  msgId; /* last used message id */
    GateMutex_Handle gate; /* message id gate */
    OsalSemaphore_Handle mbxLock; /* mailbox lock */
    OsalSemaphore_Handle queueLock; /* message queue lock */
    List_Handle recipients; /* list of waiting recipients */
    List_Handle newMail; /* list of undelivered messages */
} RcmClient_Object;

/* structure for RcmClient module state */
typedef struct RcmClient_ModuleObject_tag {
    UInt32 setupRefCount;
    /* Ref. count for number of times setup/destroy were called in this process */
    RcmClient_Config defaultCfg; /* Default config values */
    RcmClient_Params defaultInst_params; /* Default instance creation parameters */
    UInt16 heapIdAry[RCMCLIENT_HEAPID_ARRAY_LENGTH];
    /* RCM Client default heap array */
} RcmClient_ModuleObject;

/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*
 *  @var    RcmClient_state
 *
 *  @brief  RcmClient state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
RcmClient_ModuleObject RcmClient_module_obj =
{
    .defaultCfg.maxNameLen = MAX_NAME_LEN,
    .defaultCfg.defaultHeapIdArrayLength = RCMCLIENT_HEAPID_ARRAY_LENGTH,
    .defaultCfg.defaultHeapBlockSize = RCMCLIENT_HEAPID_ARRAY_BLOCKSIZE,
    .setupRefCount = 0,
    .defaultInst_params.heapId = RCMCLIENT_DEFAULT_HEAPID,
    .defaultInst_params.callbackNotification = false
};

/*
 *  @var    RcmClient_module
 *
 *  @brief  Pointer to RcmClient_module_obj .
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
RcmClient_ModuleObject* RcmClient_module = &RcmClient_module_obj;

/* =============================================================================
 *  private functions
 * =============================================================================
 */
static UInt16 genMsgId(RcmClient_Handle handle);
static inline RcmClient_Packet *getPacketAddr(RcmClient_Message *msg);
static inline RcmClient_Packet *getPacketAddrMsgqMsg(MessageQ_Msg msg);
static inline RcmClient_Packet *getPacketAddrElem(List_Elem *elem);
static Int getReturnMsg(RcmClient_Handle handle, const UInt16 msgId,
                    RcmClient_Message *returnMsg);

/*
 *  @brief      Function to get default configuration for the RcmClient module.
 *
 *  @param      cfgParams   Configuration values.
 *
 *  @sa         RcmClient_setup
 */
Int RcmClient_getConfig (RcmClient_Config *cfgParams)
{
    Int status = RCMCLIENT_SOK;

    GT_1trace (curTrace, GT_ENTER, "RcmClient_getConfig", cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        /* @retval RCMCLIENT_EINVALIDARG Argument of type
         *          (RcmClient_Config *) passed is null
         */
        status = RCMCLIENT_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_getConfig",
                             status,
                             "Argument of type (RcmClient_Config *) passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        *cfgParams = RcmClient_module->defaultCfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

     GT_1trace (curTrace, GT_LEAVE, "RcmClient_getConfig", status);

    /* @retval RCMCLIENT_SOK Operation was Successful */
    return status;
}


/*
 *  @brief     Function to setup the RcmClient module.
 *
 *  @param     config  Module configuration parameters
 *
 *  @sa        RcmClientf_destroy
 */
Int RcmClient_setup(const RcmClient_Config *config)
{
    Int status = RCMCLIENT_SOK;
    Int i;

    GT_1trace (curTrace, GT_ENTER, "RcmClient_setup", config);

    GT_assert (curTrace, (config != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (config == NULL) {
        /* @retval RCMCLIENT_EINVALIDARG
         *          Argument of type
         *          (RcmClient_Config *) passed is null
         */
        status = RCMCLIENT_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_setup",
                             status,
                             "Argument of type (Heap_Config *) passed "
                             "is null!");
    } else if (config->maxNameLen == 0) {
        /* @retval RCMCLIENT_EINVALIDARG Config->maxNameLen passed is 0 */
        status = RCMCLIENT_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_setup",
                             status,
                             "Config->maxNameLen passed is 0!");
    } else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* TBD: Protect from multiple threads. */
        RcmClient_module->setupRefCount++;

        /* This is needed at runtime so should not be in
         * SYSLINK_BUILD_OPTIMIZE.
         */
        GT_assert (curTrace, (RcmClient_module->setupRefCount < 2));
        if (RcmClient_module->setupRefCount > 1) {
            /* @retval RCMCLIENT_SALREADYSETUP Success: RcmClient module has been
             *          already setup in this process
             */
            status = RCMCLIENT_SALREADYSETUP;
            GT_1trace (curTrace,
                    GT_1CLASS,
                    "RcmClient module has been already setup in this process.\n"
                    " RefCount: [%d]\n",
                    RcmClient_module->setupRefCount);
        } else {
            for (i = 0;
                 i < RcmClient_module->defaultCfg.defaultHeapIdArrayLength;
                 i++) {
                /* TODO setup of default heaps for proc ID */
                RcmClient_module->heapIdAry[i] = 0xFFFF;
            }
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /* @retval RCMCLIENT_SOK Operation Successful */
    return status;
}


/*
 *  @brief      Function to destroy the RcmClient module.
 *
 *  @param      None
 *
 *  @sa         RcmClient_destroy
 */
Int RcmClient_destroy(void)
{
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_destroy");

    /* TBD: Protect from multiple threads. */
    RcmClient_module->setupRefCount--;

    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */

    if (RcmClient_module->setupRefCount < 0) {
        /* @retval RCMCLIENT_SALREADYCLEANEDUP:RcmClient module has been
         *          already setup in this process
         */
        status = RCMCLIENT_SALREADYCLEANEDUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "RcmClient module has been already been cleaned up in this process.\n"
                   "    RefCount: [%d]\n",
                   RcmClient_module->setupRefCount);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_destroy", status);
    /* @retval RCMCLIENT_SOK Operation Successful */
    return status;
}

/*
 *  ======== rcmclient_create ========
 * Purpose:
 * Create a RCM client instance
 */
Int RcmClient_create(String server,
             const RcmClient_Params *params,
             RcmClient_Handle *rcmclientHandle)
{
    MessageQ_Params msgQParams;
    UInt16 procId;
    RcmClient_Object *handle = NULL;
    List_Params  listParams;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_create");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_create",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == params) {
        GT_0trace(curTrace, GT_4CLASS, "RcmClient_create: invalid argument\n");
        status = RCMCLIENT_EINVALIDARG;
        goto exit;
    }

    if ((strlen(server)) > RcmClient_module->defaultCfg.maxNameLen) {
        GT_0trace(curTrace, GT_4CLASS,
            "rcmclient_create: server name too long\n");
        status = RCMCLIENT_ENAMELENGTHLIMIT;
        goto exit;
    }

    handle = (RcmClient_Object *)Memory_calloc(NULL,
                         sizeof(RcmClient_Object),
                         0);
    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_create: memory calloc failed\n");
        status = RCMCLIENT_EMEMORY;
        goto exit;
    }

    /* initialize instance data */
    handle->msgQ = NULL;
    handle->heapId = 0xFFFF;

    /* create the message queue for inbound messages */
    MessageQ_Params_init(NULL, &msgQParams);

    /* create the message queue for inbound messages */
    handle->msgQ = MessageQ_create(NULL, &msgQParams);
    if (handle->msgQ == NULL) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_create",
                     status,
                     "Unable to create MessageQ");
        status = RCMCLIENT_EOBJECT;
        goto exit;
    }

    /* locate server message queue */
    retval = MessageQ_open (server, &(handle->serverMsgQ));
    if (retval < 0) {
        if (retval == MESSAGEQ_E_NOTFOUND)
        {
            GT_0trace(curTrace, GT_4CLASS,
                "MessageQ not found\n");
            status = RCMCLIENT_ESERVER;
        }
        else
        {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "MessageQ_open",
                         retval,
                         "Error in opening MessageQ");
            status = RCMCLIENT_EOBJECT;
        }
        goto serverMsgQ_fail;
    }

    handle->cbNotify = params->callbackNotification;
    /* create callback server */
    if (handle->cbNotify == true) {
        /* TODO create callback server thread */
        /* make sure to free resources acquired by thread */
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "RcmClient asynchronous transfers not supported \n");
    }

    /* register the heapId used for message allocation */
    handle->heapId = params->heapId;
    if (RCMCLIENT_DEFAULT_HEAPID == handle->heapId) {
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "Rcm default heaps not supported \n");
        /* extract procId from the server queue id */
        //procId = MessageQ_getProcId(&(handle->serverMsgQ));
        procId = (UInt16) ((UInt32)handle->serverMsgQ) >> 16;
        /* MessageQ_getProcId needs to be fixed!*/

        if (procId >= RCMCLIENT_HEAPID_ARRAY_LENGTH) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "procID",
                         status,
                         "procId >="
                          "RCMCLIENT_HEAPID_ARRAY_LENGTH");
            status = RCMCLIENT_ERANGE;
            goto serverMsgQ_fail;
        }
        handle->heapId = RcmClient_module->heapIdAry[procId];
    }

    /* create a gate instance */
    handle->gate = GateMutex_create();
    GT_assert (curTrace, (handle->gate != NULL));
    if (handle->gate == NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "GateMutex_create",
                        status,
                        "Unable to create mutex");
        status = RCMCLIENT_EOBJECT;
        goto gate_fail;
    }

    /* create the mailbox lock */
    handle->mbxLock = OsalSemaphore_create(OsalSemaphore_Type_Counting, 1);
    GT_assert (curTrace, (handle->mbxLock != NULL));
    if (handle->mbxLock ==  NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_create",
                        status,
                        "Unable to create mbx lock");
        status = RCMCLIENT_EOBJECT;
        goto mbxLock_fail;
    }

    /* create the message queue lock */
    handle->queueLock = OsalSemaphore_create(OsalSemaphore_Type_Counting, 1);
    GT_assert (curTrace, (handle->queueLock != NULL));
    if (handle->queueLock ==  NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_create",
                        status,
                        "Unable to create queue lock");
        status = RCMCLIENT_EOBJECT;
        goto queueLock_fail;
    }

    /* create the return message recipient list */
    List_Params_init(&listParams);
    handle->recipients = List_create(&listParams);
    GT_assert (curTrace, (handle->recipients != NULL));
    if (handle->recipients == NULL) {
            GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "List_create",
                        status,
                        "Unable to create recipients list");
            status = RCMCLIENT_EOBJECT;
            goto recipients_fail;
    }

    /* create list of undelivered messages (new mail) */
    handle->newMail = List_create(&listParams);
    GT_assert (curTrace, (handle->newMail != NULL));
    if (handle->newMail == NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "List_create",
                        status,
                        "Unable to create new mail list");
        status = RCMCLIENT_EOBJECT;
        goto newMail_fail;
    }

    *rcmclientHandle = (RcmClient_Handle)handle;
    goto exit;

newMail_fail:
    retval = List_delete(&(handle->recipients));

recipients_fail:
    retval = OsalSemaphore_delete(&(handle->queueLock));
    if (retval < 0){
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     status,
                     "queue lock delete failed");
        status = RCMCLIENT_ERESOURCE;
    }

queueLock_fail:
    retval = OsalSemaphore_delete(&(handle->mbxLock));
    if (retval < 0){
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     retval,
                     "mbx lock delete failed");
        status = RCMCLIENT_ERESOURCE;
    }

mbxLock_fail:
    GateMutex_delete(&(handle->gate));

gate_fail:
    /* TODO Default heap cleanup */
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_setup", status);

serverMsgQ_fail:
    retval = MessageQ_delete(&(handle->msgQ));
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_delete",
                     retval,
                     "Unable to delete MessageQ");
        status = RCMCLIENT_ECLEANUP;
    }
    handle->msgQ = NULL;
    handle->heapId = 0xFFFF;
    Memory_free(NULL, (RcmClient_Object *)handle, sizeof(RcmClient_Object));
    handle = NULL;

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_create", status);
    return status;
}


/*
 *  ======== rcmclient_delete ========
 * Purpose:
 * Delete RCM Client instance
 */
Int RcmClient_delete(RcmClient_Handle *handlePtr)
{
    List_Elem *elem = NULL;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_delete");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_delete",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if ((NULL == handlePtr) || (NULL == *handlePtr)) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_delete: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    if ((*handlePtr)->newMail) {
        while (!(List_empty((*handlePtr)->newMail))) {
            elem = List_get((*handlePtr)->newMail);
            retval = List_remove((*handlePtr)->newMail, elem);
        }
        retval = List_delete(&((*handlePtr)->newMail));
    }
    (*handlePtr)->newMail = NULL;

    if ((*handlePtr)->recipients) {
        while (!(List_empty((*handlePtr)->recipients))) {
            elem = List_get((*handlePtr)->recipients);
            retval = List_remove((*handlePtr)->recipients, elem);
        }
        retval = List_delete(&((*handlePtr)->recipients));
    }
    (*handlePtr)->recipients = NULL;

    if ((*handlePtr)->queueLock) {
        retval = OsalSemaphore_delete(&((*handlePtr)->queueLock));
        if (retval < 0){
            GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_delete",
                        retval,
                        "queue lock delete failed");
            status = RCMCLIENT_ERESOURCE;
        }
        (*handlePtr)->queueLock = NULL;
    }

    if ((*handlePtr)->mbxLock) {
        retval = OsalSemaphore_delete(&((*handlePtr)->mbxLock));
        if (retval < 0){
            GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_delete",
                        retval,
                        "mbx lock delete failed");
            status = RCMCLIENT_ERESOURCE;
        }
        (*handlePtr)->mbxLock = NULL;
    }

    if (NULL != (*handlePtr)->gate) {
        GateMutex_delete(&((*handlePtr)->gate));
        (*handlePtr)->gate = NULL;
    }

    if ((*handlePtr)->cbNotify) {
        /* TODO delete callback server thread */
        /* free resources acquired by thread */
    }

    if ((*handlePtr)->serverMsgQ) {
        MessageQ_close(&((*handlePtr)->serverMsgQ));
        (*handlePtr)->serverMsgQ = 0xFFFFFFFF;
    }

    if ((*handlePtr)->msgQ) {
        retval = MessageQ_delete(&((*handlePtr)->msgQ));
        if (retval < 0)
    {
        GT_0trace (curTrace,
                          GT_4CLASS,
                   "RcmClient_delete: Error in MessageQ_delete\n");
        status = RCMCLIENT_ECLEANUP;
        goto exit;
        }
    }

    Memory_free(NULL, (RcmClient_Object *)*handlePtr, sizeof(RcmClient_Object));
    *handlePtr = NULL;

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_delete", status);
    return status;
}

/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         RcmClient_create
 */
Int RcmClient_Params_init(RcmClient_Handle handle,
                          RcmClient_Params *params)
{
    Int status = RCMCLIENT_SOK;

    GT_1trace (curTrace, GT_ENTER, "RcmClient_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (RcmClient_module->setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_Params_init",
                             RCMCLIENT_EINVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = RCMCLIENT_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_Params_init",
                             status,
                             "Argument of type (RcmClient_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle == NULL) {
            *params = RcmClient_module->defaultInst_params;
        } else {
            params->heapId = handle->heapId,
            params->callbackNotification = handle->cbNotify;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_Params_init", status);
    return status;
}

/*
 *  ======== RcmClient_addSymbol ========
 * Purpose:
 * Adds symbol to server, return the function index.
 */
Int RcmClient_addSymbol(RcmClient_Handle        handle,
                        String                  funcName,
                        RcmClient_RemoteFuncPtr address,
                        UInt32 *                fxn_idx)
{
    Int status = RCMCLIENT_ENOTSUPPORTED;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_addSymbol");

   if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_addSymbol",
                             status,
                             "Modules is invalidstate!");
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_addSymbol", status);
    return status;
}


/*
 *  ======== RcmClient_getHeaderSize ========
 * Purpose:
 * Returns size (in chars) of RCM header.
 */
Int RcmClient_getHeaderSize (Void)
{
    Int headerSize;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_getHeaderSize");

    /* size (in bytes) of RCM header including the messageQ header */
    /* We deduct sizeof(UInt32) as "data[1]" is the start of the payload */
    headerSize = sizeof (RcmClient_Packet) - sizeof (UInt32);

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_getHeaderSize", status);

    return headerSize;
}


/*
 *  ======== RcmClient_alloc ========
 * Purpose:
 * Allocates memory for RCM message on heap, populates MessageQ and RCM message.
 */
RcmClient_Message *RcmClient_alloc(RcmClient_Handle handle, UInt32 dataSize)
{
    Int totalSize;
    RcmClient_Packet *packet;
    RcmClient_Message *rcmMsg;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_alloc");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_alloc",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_alloc: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* total memory size (in chars) needed for headers and payload */
    /* We deduct sizeof(UInt32) as "data[1]" is the start of the payload */
    totalSize = sizeof(RcmClient_Packet) - sizeof(UInt32) + dataSize;

    /* allocate the message */
    packet = (RcmClient_Packet*)MessageQ_alloc(handle->heapId, totalSize);

    if (NULL == packet) {
        rcmMsg = NULL;
        status = RCMCLIENT_EMEMORY;
        goto exit;
    }

    /* initialize the packet structure */
    packet->desc = 0;
    packet->msgId = genMsgId(handle);
    packet->message.fxnIdx = RCMCLIENT_INVALID_FUNCTION_INDEX;
    packet->message.result = 0;
    packet->message.dataSize = dataSize;

    /* set rcmMsg pointer to start of the message struct */
    rcmMsg = (RcmClient_Message *)(&(packet->message));

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_alloc", status);
    return rcmMsg;
}


/*
 *  ======== RcmClient_exec ========
 * Purpose:
 * Requests RCM server to execute remote function
 */
Int RcmClient_exec(RcmClient_Handle handle,
           RcmClient_Message *rcmMsg)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    UInt16 msgId;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_exec");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_exec",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_exec: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(rcmMsg);
    packet->desc |= RcmClient_Desc_RCM_MSG << RCMCLIENT_DESC_TYPE_SHIFT;
    msgId = packet->msgId;

    /* set the return address to this instance's message queue */
    msgqInst = (MessageQ_Handle)handle->msgQ;
    msgqMsg = (MessageQ_Msg)packet;
    MessageQ_setReplyQueue(msgqInst, msgqMsg);

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RCMCLIENT_ESENDMSG;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, rcmMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_exec", status);
    return status;
}


/*
 *  ======== RcmClient_execAsync ========
 * Purpose:
 * Requests RCM  server to execute remote function, it is asynchronous
 */
Int RcmClient_execAsync(RcmClient_Handle handle,
             RcmClient_Message *rcmMsg,
             RcmClient_CallbackFuncPtr callback,
             Ptr appData)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execAsync");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_execAsync",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_execAsync: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* cannot use this function if callback notification is false */
    if (!handle->cbNotify) {
        status = RCMCLIENT_EASYNCENABLED;
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "!cbNotify",
                     status,
                     "Asynchronous transfer not enabled");
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(rcmMsg);
    packet->desc |= RcmClient_Desc_RCM_MSG << RCMCLIENT_DESC_TYPE_SHIFT;

    /* set the return address to this instance's message queue */
    msgqInst = (MessageQ_Handle)handle->msgQ;
    msgqMsg = (MessageQ_Msg)packet;
    MessageQ_setReplyQueue(msgqInst, msgqMsg);

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RCMCLIENT_ESENDMSG;
        goto exit;
    }

    /* TODO finish this function */

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_execAsync", status);
    return status;
}


/*
 *  ======== RcmClient_execDpc ========
 * Purpose:
 * Requests RCM server to execute remote function,
 * does not wait for completion of remote function for reply
 */
Int RcmClient_execDpc(RcmClient_Handle handle,
              RcmClient_Message *rcmMsg)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    UInt16 msgId;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execDpc");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_execDpc",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_execDpc: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(rcmMsg);
    packet->desc |= RcmClient_Desc_DPC << RCMCLIENT_DESC_TYPE_SHIFT;
    msgId = packet->msgId;

    /* set the return address to this instance's message queue */
    msgqInst = (MessageQ_Handle)handle->msgQ;
    msgqMsg = (MessageQ_Msg)packet;
    MessageQ_setReplyQueue(msgqInst, msgqMsg);

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RCMCLIENT_ESENDMSG;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, rcmMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_execDpc", status);
    return status;
}


/*
 *  ======== RcmClient_execNoWait ========
 * Purpose:
 * Requests RCM server to execute remote function,
 * provides a msgId to wait on later
 */
Int RcmClient_execNoWait(RcmClient_Handle handle,
             RcmClient_Message *rcmMsg,
             UInt16 *msgId)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execNoWait");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_execNoWait",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_execNoWait: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(rcmMsg);
    packet->desc |= RcmClient_Desc_RCM_MSG << RCMCLIENT_DESC_TYPE_SHIFT;
    *msgId = packet->msgId;

    /* set the return address to this instance's message queue */
    msgqInst = (MessageQ_Handle)handle->msgQ;
    msgqMsg = (MessageQ_Msg)packet;
    MessageQ_setReplyQueue(msgqInst, msgqMsg);

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RCMCLIENT_ESENDMSG;
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_execNoWait", status);
    return status;
}


/*
 *  ======== RcmClient_free ========
 * Purpose:
 * Frees the RCM message and allocated memory
 */
Void RcmClient_free(RcmClient_Handle handle, RcmClient_Message *rcmMsg)
{
    MessageQ_Msg msgqMsg = (MessageQ_Msg)getPacketAddr(rcmMsg);
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_free");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_free",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    MessageQ_free(msgqMsg);

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_free", status);
    return;
}


/*
 *  ======== RcmClient_getSymbolIndex ========
 * Purpose:
 * Gets symbol index
 */
Int RcmClient_getSymbolIndex(RcmClient_Handle handle,
                 String funcName,
                 UInt32 *fxnIdx)
{
    Int len;
    RcmClient_Message *rcmMsg;
    RcmClient_Packet *packet;
    UInt16 msgId;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_getSymbolIndex");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_getSymbolIndex",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_getSymbolIndex: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    *fxnIdx = RCMCLIENT_INVALID_FUNCTION_INDEX;

    /* allocate a message */
    len = strlen(funcName) + 1;
    rcmMsg = RcmClient_alloc(handle, len);

    /* copy the function name into the message payload */
    rcmMsg->dataSize = len;  //TODO this is not proper!
    strcpy((Char *)rcmMsg->data, funcName);

    /* classify this message */
    packet = getPacketAddr(rcmMsg);
    packet->desc |= RcmClient_Desc_SYM_IDX << RCMCLIENT_DESC_TYPE_SHIFT;
    msgId = packet->msgId;

    /* set the return address to this instance's message queue */
    msgqInst = (MessageQ_Handle)handle->msgQ;
    msgqMsg = (MessageQ_Msg)packet;
    MessageQ_setReplyQueue(msgqInst, msgqMsg);

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RCMCLIENT_ESENDMSG;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, rcmMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
        goto exit;
    }

    /* extract return value and free message */
    packet = getPacketAddr(rcmMsg);
    *fxnIdx = (UInt32)packet->message.result;
    RcmClient_free(handle, rcmMsg);

    /* raise an error if the symbol was not found */
    if (RCMCLIENT_INVALID_FUNCTION_INDEX == *fxnIdx) {
        status = RCMCLIENT_ESYMBOLNOTFOUND;
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "symbol not found",
                     status,
                     "symbol not found");
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_getSymbolIndex", status);
    return status;
}


/*
 *  ======== RcmClient_removeSymbol ========
 * Purpose:
 * Removes symbol (remote function) from registry
 */
Int RcmClient_removeSymbol(RcmClient_Handle handle, String funcName)
{
    Int status = RCMCLIENT_ENOTSUPPORTED;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_removeSymbol");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_removeSymbol",
                             status,
                             "Modules is invalidstate!");
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_removeSymbol", status);
    return status;
}

/*
 *  ======== RcmClient_waitUntilDone ========
 * Purpose:
 * Waits till invoked remote function completes
 * return message will contain result and context,
 */
Int RcmClient_waitUntilDone(RcmClient_Handle handle,
                UInt16 msgId,
                RcmClient_Message *rcmMsg)
{
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_waitUntilDone");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_waitUntilDone",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_waitUntilDone: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, rcmMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
        goto exit;
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_waitUntilDone", status);
    return status;
}


/*
 *  ======== genMsgId ========
 * Purpose:
 * To generate unique MsgIDs for the RCM messages
 */
UInt16 genMsgId(RcmClient_Handle handle)
{
    UInt32 key;
    UInt16 msgId;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "genMsgId");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "genMsgId",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "genMsgId: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    /* generate a new message id */
    key = GateMutex_enter(handle->gate);
    msgId = (handle->msgId == 0xFFFF ? handle->msgId = 1
                        : ++(handle->msgId));
    GateMutex_leave(handle->gate, key);

exit:
    GT_1trace (curTrace, GT_LEAVE, "genMsgId", status);
    return msgId;
}


/*
 *  ======== getReturnMsg ========
 * Purpose:
 *  A thread safe algorithm for message delivery
 *
 *  This function is called to pickup a specified return message from
 *  the server. messages are taken from the queue and either delivered
 *  to the mailbox if not the specified message or returned to the caller.
 *  The calling thread might play the role of mailman or simply wait
 *  on a list of recipients.
 *
 *  This algorithm guarantees that a waiting recipient is released as soon
 *  as his message arrives. All new mail is placed in the mailbox until the
 *  recipient arrives to collect it. The messages can arrive in any order
 *  and will be processed as they arrive. message delivery is never stalled
 *  waiting on an absent recipient.
 */
Int getReturnMsg(RcmClient_Handle handle,
                 const UInt16 msgId,
                 RcmClient_Message *returnMsg)
{
    List_Elem *elem;
    Recipient *recipient;
    RcmClient_Packet *packet;
    Bool messageDelivered;
    MessageQ_Msg msgqMsg = NULL;
    MessageQ_Handle msgQ = (MessageQ_Handle)handle->msgQ;
    Bool messageFound = FALSE;
    Int queueLockAcquired = 0;
    Int retval = 0;
    Int status = RCMCLIENT_SOK;

    GT_0trace (curTrace, GT_ENTER, "getReturnMsg");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMCLIENT_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "getReturnMsg",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_waitUntilDone: invalid argument\n");
        status = RCMCLIENT_EHANDLE;
        goto exit;
    }

    returnMsg = NULL;

    /* keep trying until message found */
    while (!messageFound) {

        /* acquire the mailbox lock */
        retval = OsalSemaphore_pend(handle->mbxLock, OSALSEMAPHORE_WAIT_FOREVER);
        if (retval < 0) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "OsalSemaphore_pend",
                         retval,
                         "handle->mbxLock pend fails");
            status = RCMCLIENT_ERESOURCE;
            goto exit;
        }

        /* search new mail list for message */
        elem = NULL;
        while ((elem = List_next(handle->newMail, elem)) != NULL) {
            packet = getPacketAddrElem(elem);
            if (msgId == packet->msgId) {
                retval = List_remove(handle->newMail, elem);
                if (retval < 0) {
                    GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "List_remove",
                                 retval,
                                 "unable to remove"
                                      "element");
                    status = RCMCLIENT_ERESOURCE;
                    goto exit;
                }
                returnMsg = &packet->message;
                messageFound = TRUE;
                retval = OsalSemaphore_post(handle->mbxLock);
                if (retval < 0) {
                    GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalSemphore post",
                                 retval,
                                 "handle->mbxLock post fails");
                    status = RCMCLIENT_ERESOURCE;
                    goto exit;
                }
                goto exit;
            }
        }

        /* attempt the message queue lock */
        queueLockAcquired = OsalSemaphore_pend(handle->queueLock,
                                    OSALSEMAPHORE_WAIT_NONE);
        if ((queueLockAcquired <0) && (queueLockAcquired !=
                        OSALSEMAPHORE_E_WAITNONE)) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "OsalSemaphore_pend",
                         status,
                         "handle->queueLock fails");
            status = RCMCLIENT_ERESOURCE;
            goto exit;
        }

        if (! (queueLockAcquired < 0)) {
            /*
             * mailman role
             */

            /* deliver new mail until message found */
            while (!messageFound) {
                /* get message from queue if available (non-blocking) */
                if (NULL == msgqMsg) {
                    retval = MessageQ_get(msgQ, &msgqMsg, WAIT_NONE);
                    if ((retval < 0) && (retval != MESSAGEQ_E_TIMEOUT)) {
                        GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_get",
                                     retval,
                                     "handle->MessageQ get fails");
                        status = RCMCLIENT_EGETMSG;
                        goto exit;
                    }
                }

                while (NULL != msgqMsg) {
                    /* check if message found */
                    packet = getPacketAddrMsgqMsg(msgqMsg);
                    messageFound = (msgId == packet->msgId);
                    if (messageFound) {
                        returnMsg = &packet->message;

                        /* search wait list for new mailman */
                        elem = NULL;
                        while ((elem = List_next(handle->recipients, elem)) != NULL) {
                            recipient = (Recipient *)elem;
                            if (NULL == recipient->msg) {
                                retval = OsalSemaphore_post(recipient->event);
                                if (retval < 0) {
                                    GT_setFailureReason (curTrace,
                                                 GT_4CLASS,
                                                 "Osal Semaphore post",
                                                 retval,
                                                 "recipient->event post fails");
                                    status = RCMCLIENT_ERESOURCE;
                                    goto exit;
                                }
                                break;
                            }
                        }

                        /* release the message queue lock */
                        retval = OsalSemaphore_post(handle->queueLock);
                        if (retval < 0) {
                            GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "Osal Semaphore post",
                                         retval,
                                         "handle->queueLock post fails");
                            status = RCMCLIENT_ERESOURCE;
                            goto exit;
                        }

                        /* release the mailbox lock */
                        retval = OsalSemaphore_post(handle->mbxLock);
                        if (retval < 0) {
                            GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "Osal Semaphore post",
                                         retval,
                                         "handle->mbxLock post fails");
                            status = RCMCLIENT_ERESOURCE;
                        }
                        goto exit;
                    }

                    /*
                     * deliver message to mailbox
                     */

                    /* search recipient list for message owner */
                    elem = NULL;
                    messageDelivered = FALSE;
                    while ((elem = List_next
                        (handle->recipients, elem)) != NULL) {
                        recipient = (Recipient *)elem;
                        if (recipient->msgId == packet->msgId) {
                            recipient->msg = &packet->message;
                            retval = OsalSemaphore_post(recipient->event);
                            if (retval < 0) {
                                GT_setFailureReason (curTrace,
                                             GT_4CLASS,
                                             "Osal Semaphore post",
                                             retval,
                                             "recipient->event post fails");
                                status = RCMCLIENT_ERESOURCE;
                                goto exit;
                            }
                            messageDelivered = TRUE;
                            break;
                        }
                    }

                    /* add undelivered message to new mail list */
                    if (!messageDelivered) {
                        /* use the elem in the MessageQ hdr */
                        elem = (List_Elem *)&packet->msgqHeader;
                        retval = List_put
                            (handle->newMail, elem);
                        if (retval < 0) {
                            GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "List_put",
                                         retval,
                                         "New mail List_put error");
                            status = RCMCLIENT_ERESOURCE;
                            goto exit;
                        }
                    }
                    retval = MessageQ_get(msgQ, &msgqMsg, WAIT_NONE);
                    if ((retval < 0) && (retval != MESSAGEQ_E_TIMEOUT)) {
                        GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_get",
                                     retval,
                                     "handle->MessageQ get fails");
                        status = RCMCLIENT_EGETMSG;
                        goto exit;
                    }
                }

                /*
                 * message queue empty
                 */

                /* release the mailbox lock */
                retval = OsalSemaphore_post(handle->mbxLock);
                if (retval < 0) {
                    GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalSemaphore_post",
                                 retval,
                                 "uhandle->mbxLock"
                                    "post fails");
                    status = RCMCLIENT_ERESOURCE;
                    goto exit;
                }

                /* get next message, this blocks the thread */
                retval = MessageQ_get(msgQ, &msgqMsg, MESSAGEQ_FOREVER);
                 if (retval < 0) {
                      GT_setFailureReason (curTrace,
                                    GT_4CLASS,
                                    "MessageQ_get",
                                    retval,
                                    "handle->MessageQ get fails");
                       status = RCMCLIENT_EGETMSG;
                       goto exit;
                   }

                if (msgqMsg == NULL) {
                    GT_0trace(curTrace, GT_4CLASS,
                        "get_return_msg: msgq_msg == NULL\n");
                    status = RCMCLIENT_EGETMSG;
                    goto exit;
                }

                /* acquire the mailbox lock */
                retval = OsalSemaphore_pend(handle->mbxLock,
                                OSALSEMAPHORE_WAIT_FOREVER);
                if (retval < 0) {
                    GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalSemaphore_pend",
                                 retval,
                                 "handle->queueLock fails");
                    status = RCMCLIENT_ERESOURCE;
                }
            }
        } else {
            /* construct recipient on local stack */
            Recipient self;
            self.msgId = msgId;
            self.msg = NULL;
            self.event = OsalSemaphore_create(OsalSemaphore_Type_Counting, 0);
            if (self.event ==  NULL) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_create",
                             status,
                             "thread event"
                             "construct fails");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }

            /* add recipient to wait list */
            elem = (List_Elem *)&self;
            List_put(handle->recipients, elem);

            /* release the mailbox lock */
            retval = OsalSemaphore_post(handle->mbxLock);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             retval,
                             "handle->mbxLock"
                             "post fails");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }

            /* wait on event */
            retval = OsalSemaphore_pend(self.event,
                        OSALSEMAPHORE_WAIT_FOREVER);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_pend",
                             retval,
                             "thread event pend fails");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }

            /* acquire the mailbox lock */
            retval = OsalSemaphore_pend(handle->mbxLock,
                            OSALSEMAPHORE_WAIT_FOREVER);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_pend",
                             retval,
                             "handle->mbxLock pend fails");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }

            if (NULL != self.msg) {
                /* pickup message */
                returnMsg = self.msg;
                messageFound = TRUE;
            }

            /* remove recipient from wait list */
            retval = List_remove(handle->recipients, elem);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_remove",
                             retval,
                             "recipients List"
                            "remove error");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }
            retval = OsalSemaphore_delete(&(self.event));
            if (retval < 0){
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_delete",
                             retval,
                             "thread event delete failed");
                status = RCMCLIENT_ERESOURCE;
                goto exit;
            }

            /* release the mailbox lock */
            retval = OsalSemaphore_post(handle->mbxLock);
            if (retval < 0){
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             retval,
                             "handle->mbxLock post fails");
                status = RCMCLIENT_ERESOURCE;
            }
        }
    }
exit:
    GT_1trace (curTrace, GT_LEAVE, "getReturnMsg", status);
    return status;
}

/*
 *  ======== getPacketAddr ========
 * Purpose:
 * Gets packet address from RCM message
 */
inline RcmClient_Packet *getPacketAddr(RcmClient_Message *msg)
{
    Int offset = (Int)&(((RcmClient_Packet *)0)->message);
    return ((RcmClient_Packet *)((Char *)msg - offset));
}

/*
 *  ======== getPacketAddrMsgqMsg ========
 * Purpose:
 * Gets packet address from MessageQ message
 */
inline RcmClient_Packet *getPacketAddrMsgqMsg(MessageQ_Msg msg)
{
    Int offset = (Int)&(((RcmClient_Packet *)0)->msgqHeader);
    return ((RcmClient_Packet *)((Char *)msg - offset));
}

/*
 *  ======== getPacketAddrElem ========
 * Purpose:
 * Gets packet address from list element
 */
inline RcmClient_Packet *getPacketAddrElem(List_Elem *elem)
{
    Int offset = (Int)&(((RcmClient_Packet *)0)->msgqHeader);
    return ((RcmClient_Packet *)((Char *)elem - offset));
}
