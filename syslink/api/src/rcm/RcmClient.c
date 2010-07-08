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
#include <ti/ipc/MessageQ.h>
#include <GateMutex.h>
#include <IGateProvider.h>
#include <OsalSemaphore.h>
#include <Memory.h>
#include <List.h>

/* Module level headers */
#include <RcmTypes.h>
#include <RcmClient.h>

/* =============================================================================
 * RCM client defines
 * =============================================================================
 */
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
    IGateProvider_Handle gate; /* message id gate */
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
                    RcmClient_Message **returnMsg);

/*
 *  ======== RcmClient_init ========
 * Purpose:
 * Setup RCM client module
 */
Void RcmClient_init(Void)
{
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_init");

    /* TBD: Protect from multiple threads. */
    RcmClient_module->setupRefCount++;

    /* This is needed at runtime so should not be in
     * SYSLINK_BUILD_OPTIMIZE.
     */
    if (RcmClient_module->setupRefCount > 1) {
        /* @retval RcmClient_S_ALREADYSETUP Success: RcmClient module has been
         *          already setup in this process
         */
        status = RcmClient_S_ALREADYSETUP;
        GT_1trace (curTrace,
                GT_1CLASS,
                "RcmClient module has been already setup in this process.\n"
                " RefCount: [%d]\n",
                RcmClient_module->setupRefCount);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_init", status);
}

/*
 *  ======== RcmClient_exit ========
 * Purpose:
 * Clean up RCM Client module
 */
Void RcmClient_exit(Void)
{
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_exit");

    /* TBD: Protect from multiple threads. */
    RcmClient_module->setupRefCount--;

    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (RcmClient_module->setupRefCount < 0) {
        /* @retval RcmClient_S_ALREADYCLEANEDUP:RcmClient module has been
         *          already setup in this process
         */
        status = RcmClient_S_ALREADYCLEANEDUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "RcmClient module has been already been cleaned up in this process.\n"
                   "    RefCount: [%d]\n",
                   RcmClient_module->setupRefCount);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_exit", status);
}

/*
 *  ======== RcmClient_create ========
 * Purpose:
 * Create a RCM client instance
 */
Int RcmClient_create(String server,
             RcmClient_Params *params,
             RcmClient_Handle *handle)
{
    MessageQ_Params msgQParams;
    UInt16 procId;
    RcmClient_Object *obj = NULL;
    List_Params  listParams;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_create");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_create",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == params) {
        GT_0trace(curTrace, GT_4CLASS, "RcmClient_create: invalid argument\n");
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    if ((strlen(server)) > RcmClient_module->defaultCfg.maxNameLen) {
        GT_0trace(curTrace, GT_4CLASS,
            "rcmclient_create: server name too long\n");
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    obj = (RcmClient_Object *)Memory_calloc(NULL,
                         sizeof(RcmClient_Object),
                         0);
    if (NULL == obj) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_create: memory calloc failed\n");
        status = RcmClient_E_NOMEMORY;
        goto exit;
    }

    /* initialize instance data */
    obj->heapId = 0xFFFF;
    obj->msgId = 0xFFFF;
    obj->serverMsgQ = MessageQ_INVALIDMESSAGEQ;
    obj->msgQ = NULL;
    obj->mbxLock = NULL;
    obj->queueLock = NULL;
    obj->recipients = NULL;
    obj->newMail = NULL;

    /* create the message queue for inbound messages */
    MessageQ_Params_init(&msgQParams);

    /* create the message queue for inbound messages */
    obj->msgQ = MessageQ_create(NULL, &msgQParams);
    if (obj->msgQ == NULL) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_create",
                     status,
                     "Unable to create MessageQ");
        status = RcmClient_E_MSGQCREATEFAILED;
        goto exit;
    }

    /* locate server message queue */
    retval = MessageQ_open (server, &(obj->serverMsgQ));
    if (retval < 0) {
        if (retval == MessageQ_E_NOTFOUND)
        {
            GT_0trace(curTrace, GT_4CLASS,
                "MessageQ not found\n");
            status = RcmClient_E_SERVERNOTFOUND;
        }
        else
        {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "MessageQ_open",
                         retval,
                         "Error in opening MessageQ");
            status = RcmClient_E_MSGQOPENFAILED;
        }
        goto serverMsgQ_fail;
    }

    obj->cbNotify = params->callbackNotification;
    /* create callback server */
    if (obj->cbNotify == true) {
        /* TODO create callback server thread */
        /* make sure to free resources acquired by thread */
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "RcmClient asynchronous transfers not supported \n");
    }

    /* register the heapId used for message allocation */
    obj->heapId = params->heapId;
    if (RCMCLIENT_DEFAULT_HEAPID == obj->heapId) {
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "Rcm default heaps not supported \n");
        /* extract procId from the server queue id */
        //procId = MessageQ_getProcId(&(obj->serverMsgQ));
        procId = (UInt16) ((UInt32)obj->serverMsgQ) >> 16;
        /* MessageQ_getProcId needs to be fixed!*/

        if (procId >= RCMCLIENT_HEAPID_ARRAY_LENGTH) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "procID",
                         status,
                         "procId >="
                          "RCMCLIENT_HEAPID_ARRAY_LENGTH");
            status = RcmClient_E_FAIL;
            goto serverMsgQ_fail;
        }
        obj->heapId = RcmClient_module->heapIdAry[procId];
    }

    /* create a gate instance */
    obj->gate = (IGateProvider_Handle) GateMutex_create ();
    GT_assert (curTrace, (obj->gate != NULL));
    if (obj->gate == NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "GateMutex_create",
                        status,
                        "Unable to create mutex");
        status = RcmClient_E_FAIL;
        goto gate_fail;
    }

    /* create the mailbox lock */
    obj->mbxLock = OsalSemaphore_create(OsalSemaphore_Type_Counting, 1);
    GT_assert (curTrace, (obj->mbxLock != NULL));
    if (obj->mbxLock ==  NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_create",
                        status,
                        "Unable to create mbx lock");
        status = RcmClient_E_FAIL;
        goto mbxLock_fail;
    }

    /* create the message queue lock */
    obj->queueLock = OsalSemaphore_create(OsalSemaphore_Type_Counting, 1);
    GT_assert (curTrace, (obj->queueLock != NULL));
    if (obj->queueLock ==  NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_create",
                        status,
                        "Unable to create queue lock");
        status = RcmClient_E_FAIL;
        goto queueLock_fail;
    }

    /* create the return message recipient list */
    List_Params_init(&listParams);
    obj->recipients = List_create(&listParams);
    GT_assert (curTrace, (obj->recipients != NULL));
    if (obj->recipients == NULL) {
            GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "List_create",
                        status,
                        "Unable to create recipients list");
            status = RcmClient_E_LISTCREATEFAILED;
            goto recipients_fail;
    }

    /* create list of undelivered messages (new mail) */
    obj->newMail = List_create(&listParams);
    GT_assert (curTrace, (obj->newMail != NULL));
    if (obj->newMail == NULL) {
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "List_create",
                        status,
                        "Unable to create new mail list");
        status = RcmClient_E_LISTCREATEFAILED;
        goto newMail_fail;
    }

    *handle = (RcmClient_Handle)obj;
    goto exit;

newMail_fail:
    List_delete(&(obj->recipients));

recipients_fail:
    retval = OsalSemaphore_delete(&(obj->queueLock));
    if (retval < 0){
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     status,
                     "queue lock delete failed");
        status = RcmClient_E_FAIL;
    }

queueLock_fail:
    retval = OsalSemaphore_delete(&(obj->mbxLock));
    if (retval < 0){
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     retval,
                     "mbx lock delete failed");
        status = RcmClient_E_FAIL;
    }

mbxLock_fail:
    GateMutex_delete((GateMutex_Handle *)&(obj->gate));

gate_fail:
    /* TODO Default heap cleanup */
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_setup", status);

serverMsgQ_fail:
    retval = MessageQ_delete(&(obj->msgQ));
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_delete",
                     retval,
                     "Unable to delete MessageQ");
        status = RcmClient_E_FAIL;
    }
    obj->msgQ = NULL;
    obj->heapId = 0xFFFF;
    Memory_free(NULL, (RcmClient_Object *)obj, sizeof(RcmClient_Object));
    obj = NULL;

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
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_delete");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    if ((*handlePtr)->newMail) {
        while (!(List_empty((*handlePtr)->newMail))) {
            elem = List_get((*handlePtr)->newMail);
            List_remove((*handlePtr)->newMail, elem);
        }
        List_delete(&((*handlePtr)->newMail));
    }
    (*handlePtr)->newMail = NULL;

    if ((*handlePtr)->recipients) {
        while (!(List_empty((*handlePtr)->recipients))) {
            elem = List_get((*handlePtr)->recipients);
            List_remove((*handlePtr)->recipients, elem);
        }
        List_delete(&((*handlePtr)->recipients));
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
            status = RcmClient_E_FAIL;
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
            status = RcmClient_E_FAIL;
        }
        (*handlePtr)->mbxLock = NULL;
    }

    if (NULL != (*handlePtr)->gate) {
        GateMutex_delete((GateMutex_Handle *)&((*handlePtr)->gate));
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
        status = RcmClient_E_FAIL;
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
Int RcmClient_Params_init(RcmClient_Params *params)
{
    Int status = RcmClient_S_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "RcmClient_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (RcmClient_module->setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_Params_init",
                             RcmClient_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = RcmClient_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_Params_init",
                             status,
                             "Argument of type (RcmClient_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        params->heapId = RcmClient_module->defaultInst_params.heapId;
        params->callbackNotification = \
                    RcmClient_module->defaultInst_params.callbackNotification;
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
                        String                  name,
                        RcmClient_RemoteFuncPtr addr,
                        UInt32 *                index)
{
    Int status = RcmClient_E_NOTSUPPORTED;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_addSymbol");

   if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_getHeaderSize", headerSize);

    return headerSize;
}


/*
 *  ======== RcmClient_alloc ========
 * Purpose:
 * Allocates memory for RCM message on heap, populates MessageQ and RCM message.
 */
Int RcmClient_alloc(RcmClient_Handle handle, UInt32 dataSize,
    RcmClient_Message **message)
{
    Int totalSize;
    RcmClient_Packet *packet;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_alloc");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* total memory size (in chars) needed for headers and payload */
    /* We deduct sizeof(UInt32) as "data[1]" is the start of the payload */
    totalSize = sizeof(RcmClient_Packet) - sizeof(UInt32) + dataSize;

    /* allocate the message */
    packet = (RcmClient_Packet*)MessageQ_alloc(handle->heapId, totalSize);

    if (NULL == packet) {
        *message = NULL;
        status = RcmClient_E_MSGALLOCFAILED;
        goto exit;
    }

    /* initialize the packet structure */
    packet->desc = 0;
    packet->msgId = genMsgId(handle);
    packet->message.fxnIdx = RcmClient_INVALIDFXNIDX;
    packet->message.result = 0;
    packet->message.dataSize = dataSize;

    /* set cmdMsg pointer to start of the message struct */
    *message = (RcmClient_Message *)(&(packet->message));

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_alloc", status);
    return status;
}


/*
 *  ======== RcmClient_exec ========
 * Purpose:
 * Requests RCM server to execute remote function
 */
Int RcmClient_exec(RcmClient_Handle handle,
           RcmClient_Message *cmdMsg, RcmClient_Message **returnMsg)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    RcmClient_Message *rtnMsg;
    UInt16 msgId;
    UInt16 serverStatus;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_exec");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
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
        status = RcmClient_E_EXECFAILED;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, &rtnMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
        *returnMsg = NULL;
        goto exit;
    }
    *returnMsg = rtnMsg;

    /* check the server's status stored in the packet header */
    packet = getPacketAddr(rtnMsg);
    serverStatus = (RcmClient_Desc_TYPE_MASK & packet->desc) >>
            RcmClient_Desc_TYPE_SHIFT;

    switch (serverStatus) {
        case RcmServer_Status_SUCCESS:
            break;

        case RcmServer_Status_INVALID_FXN:
            GT_setFailureReason (curTrace,
                GT_4CLASS,
                "RcmClient_exec",
                serverStatus,
                "Invalid function index");
            status = RcmClient_E_INVALIDFXNIDX;
            goto exit;

        case RcmServer_Status_MSG_FXN_ERR:
            GT_setFailureReason (curTrace,
                GT_4CLASS,
                "RcmClient_exec",
                serverStatus,
                "Message function error");
            status = RcmClient_E_MSGFXNERR;
            goto exit;

        default:
            GT_setFailureReason (curTrace,
                GT_4CLASS,
                "RcmClient_exec",
                serverStatus,
                "The server returned error");
            status = RcmClient_E_SERVERERROR;
            goto exit;
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
             RcmClient_Message *cmdMsg,
             RcmClient_CallbackFxn callback,
             Ptr appData)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execAsync");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* cannot use this function if callback notification is false */
    if (!handle->cbNotify) {
        status = RcmClient_E_EXECASYNCNOTENABLED;
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "!cbNotify",
                     status,
                     "Asynchronous transfer not enabled");
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
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
        status = RcmClient_E_EXECFAILED;
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
              RcmClient_Message *cmdMsg, RcmClient_Message **returnMsg)
{
    RcmClient_Packet *packet;
    RcmClient_Message *rtnMsg;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    UInt16 msgId;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execDpc");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
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
        status = RcmClient_E_EXECFAILED;
        goto exit;
    }

    /* get the return message from the server */
    status = getReturnMsg(handle, msgId, &rtnMsg);
    if (status < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
        *returnMsg = NULL;
        goto exit;
    }
    *returnMsg = rtnMsg;

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
             RcmClient_Message *cmdMsg,
             UInt16 *msgId)
{
    RcmClient_Packet *packet;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execNoWait");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
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
        status = RcmClient_E_EXECFAILED;
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_execNoWait", status);
    return status;
}


/*
 *  ======== RcmClient_execNoReply ========
 * Purpose:
 * Requests RCM server to execute remote function,
 * without waiting for the reply.
 */
Int RcmClient_execNoReply(RcmClient_Handle handle,
             RcmClient_Message *cmdMsg)
{
    RcmClient_Packet *packet;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_execNoReply");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_execNoReply",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmClient_execNoWait: invalid argument\n");
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
    packet->desc |= RcmClient_Desc_RCM_NO_REPLY << RCMCLIENT_DESC_TYPE_SHIFT;

    msgqMsg = (MessageQ_Msg)packet;

    /* send the message to the server */
    retval = MessageQ_put(handle->serverMsgQ, msgqMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_put",
                     retval,
                     "Message put fails");
        status = RcmClient_E_EXECFAILED;
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_execNoReply", status);
    return status;
}


/*
 *  ======== RcmClient_free ========
 * Purpose:
 * Frees the RCM message and allocated memory
 */
Int RcmClient_free(RcmClient_Handle handle, RcmClient_Message *cmdMsg)
{
    Int retval;
    MessageQ_Msg msgqMsg = (MessageQ_Msg)getPacketAddr(cmdMsg);
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_free");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmClient_free",
                             status,
                             "Modules is invalidstate!");
        goto exit;
    }

    retval = MessageQ_free(msgqMsg);

    if (retval < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_free",
                             status,
                             "MessageQ free failed");
        status = RcmClient_E_IPCERR;
        goto exit;
    }


exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmClient_free", status);
    return status;
}


/*
 *  ======== RcmClient_getSymbolIndex ========
 * Purpose:
 * Gets symbol index
 */
Int RcmClient_getSymbolIndex(RcmClient_Handle handle,
                 String name,
                 UInt32 *index)
{
    Int len;
    RcmClient_Message *cmdMsg = NULL;
    RcmClient_Packet *packet;
    UInt16 msgId;
    MessageQ_Handle msgqInst;
    MessageQ_Msg msgqMsg;
    Int retval = 0;
    UInt16 serverStatus;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_getSymbolIndex");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    *index = RcmClient_E_INVALIDFXNIDX;

    /* allocate a message */
    len = strlen(name) + 1;
    retval = RcmClient_alloc(handle, len, &cmdMsg);
    if (retval < 0) {
        Osal_printf ("Error allocating RCM message\n");
        status = retval;
        goto exit;
    }

    /* copy the function name into the message payload */
    cmdMsg->dataSize = len;  //TODO this is not proper!
    strcpy((Char *)cmdMsg->data, name);

    /* classify this message */
    packet = getPacketAddr(cmdMsg);
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
        status = RcmClient_E_EXECFAILED;
        goto exit;
    }

    /* get the return message from the server */
    retval = getReturnMsg(handle, msgId, &cmdMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     status,
                     "Get return message failed");
        status = retval;
        goto exit;
    }

    /* extract return value and free message */
    packet = getPacketAddr(cmdMsg);

    serverStatus = (RcmClient_Desc_TYPE_MASK & packet->desc) >>
            RcmClient_Desc_TYPE_SHIFT;
    switch (serverStatus) {
        case RcmServer_Status_SUCCESS:
            break;

        case RcmServer_Status_SYMBOL_NOT_FOUND:
            GT_setFailureReason (curTrace,
                GT_4CLASS,
                "RcmClient_getSymbolIndex",
                serverStatus,
                "Given symbol not found");
            status = RcmClient_E_SYMBOLNOTFOUND;
            goto exit;

        default:
            GT_setFailureReason (curTrace,
                GT_4CLASS,
                "RcmClient_getSymbolIndex",
                serverStatus,
                "Server returned error");
            status = RcmClient_E_SERVERERROR;
            goto exit;
    }
    /* extract return value */
    *index = cmdMsg->data[0];

exit:
    if (cmdMsg != NULL) {
        RcmClient_free(handle, cmdMsg);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmClient_getSymbolIndex", status);
    return status;
}


/*
 *  ======== RcmClient_removeSymbol ========
 * Purpose:
 * Removes symbol (remote function) from registry
 */
Int RcmClient_removeSymbol(RcmClient_Handle handle, String name)
{
    Int status = RcmClient_E_NOTSUPPORTED;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_removeSymbol");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
                RcmClient_Message **returnMsg)
{
    Int status = RcmClient_S_SUCCESS;
    Int retval = 0;
    RcmClient_Message *rtnMsg = NULL;

    GT_0trace (curTrace, GT_ENTER, "RcmClient_waitUntilDone");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* get the return message from the server */
    retval = getReturnMsg(handle, msgId, &rtnMsg);
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "getReturnMsg",
                     retval,
                     "Get return message failed");
        *returnMsg = NULL;
        retval = status;
        goto exit;
    }
    *returnMsg = rtnMsg;

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
    IArg key;
    /* FIXME: (KW) ADD Check for msgID  = 0 in the calling function */
    UInt16 msgId = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "genMsgId");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    /* generate a new message id */
    key = IGateProvider_enter (handle->gate);
    msgId = (handle->msgId == 0xFFFF ? handle->msgId = 1
                        : ++(handle->msgId));
    IGateProvider_leave (handle->gate, key);

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
                 RcmClient_Message **returnMsg)
{
    List_Elem *elem;
    Recipient *recipient;
    RcmClient_Packet *packet;
    Bool messageDelivered;
    MessageQ_Msg msgqMsg = NULL;
    MessageQ_Handle msgQ = NULL;
    Bool messageFound = FALSE;
    Int queueLockAcquired = 0;
    Int retval = 0;
    Int status = RcmClient_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "getReturnMsg");

    if (RcmClient_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmClient_E_INVALIDSTATE;
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
        status = RcmClient_E_INVALIDARG;
        goto exit;
    }

    msgQ = (MessageQ_Handle)handle->msgQ;
    *returnMsg = NULL;

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
            status = RcmClient_E_FAIL;
            goto exit;
        }

        /* search new mail list for message */
        elem = NULL;
        while ((elem = List_next(handle->newMail, elem)) != NULL) {
            packet = getPacketAddrElem(elem);
            if (msgId == packet->msgId) {
                List_remove(handle->newMail, elem);
                *returnMsg = &packet->message;
                messageFound = TRUE;
                retval = OsalSemaphore_post(handle->mbxLock);
                if (retval < 0) {
                    GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalSemphore post",
                                 retval,
                                 "handle->mbxLock post fails");
                    status = RcmClient_E_FAIL;
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
            status = RcmClient_E_FAIL;
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
                    if ((retval < 0) && (retval != MessageQ_E_TIMEOUT)) {
                        GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_get",
                                     retval,
                                     "handle->MessageQ get fails");
                        status = RcmClient_E_LOSTMSG;
                        goto exit;
                    }
                }

                while (NULL != msgqMsg) {
                    /* check if message found */
                    packet = getPacketAddrMsgqMsg(msgqMsg);
                    messageFound = (msgId == packet->msgId);
                    if (messageFound) {
                        *returnMsg = &packet->message;

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
                                    status = RcmClient_E_FAIL;
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
                            status = RcmClient_E_FAIL;
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
                            status = RcmClient_E_FAIL;
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
                                status = RcmClient_E_FAIL;
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
                        List_put (handle->newMail, elem);
                    }

                        /* get next message from queue if available */
                    retval = MessageQ_get(msgQ, &msgqMsg, WAIT_NONE);
                    if ((retval < 0) && (retval != MessageQ_E_TIMEOUT)) {
                        GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_get",
                                     retval,
                                     "handle->MessageQ get fails");
                        status = RcmClient_E_LOSTMSG;
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
                    status = RcmClient_E_FAIL;
                    goto exit;
                }

                /* get next message, this blocks the thread */
                retval = MessageQ_get(msgQ, &msgqMsg, MessageQ_FOREVER);
                 if (retval < 0) {
                      GT_setFailureReason (curTrace,
                                    GT_4CLASS,
                                    "MessageQ_get",
                                    retval,
                                    "handle->MessageQ get fails");
                       status = RcmClient_E_LOSTMSG;
                       goto exit;
                   }

                if (msgqMsg == NULL) {
                    GT_0trace(curTrace, GT_4CLASS,
                        "get_return_msg: msgq_msg == NULL\n");
                    status = RcmClient_E_LOSTMSG;
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
                    status = RcmClient_E_FAIL;
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
                status = RcmClient_E_FAIL;
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
                status = RcmClient_E_FAIL;
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
                status = RcmClient_E_FAIL;
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
                status = RcmClient_E_FAIL;
                goto exit;
            }

            if (NULL != self.msg) {
                /* pickup message */
                *returnMsg = self.msg;
                messageFound = TRUE;
            }

            /* remove recipient from wait list */
            List_remove(handle->recipients, elem);
#if !defined(__linux)
            /* Android bionic Semdelete code returns -1 if count = 0 */
            retval = OsalSemaphore_post(self.event);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             retval,
                             "self.event"
                             "post fails");
                status = RcmClient_E_FAIL;
                goto exit;
            }
#endif
            retval = OsalSemaphore_delete(&(self.event));
            if (retval < 0){
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_delete",
                             retval,
                             "thread event delete failed");
                status = RcmClient_E_FAIL;
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
                status = RcmClient_E_FAIL;
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
