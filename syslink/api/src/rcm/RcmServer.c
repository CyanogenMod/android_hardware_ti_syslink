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
 * RcmServer.c
 *
 * The RCM server module receives RCM messages from the RCM client,
 * executes remote function and sends replies to the RCM client.
 *
 */

/*
 *  ======== RcmServer.c ========
 *  Notes:
 *  NA
 */

/* Standard headers */
#include <host_os.h>
#include <pthread.h>

/* Utility and IPC headers */
#include <Std.h>
#include <Trace.h>
#include <ti/ipc/MessageQ.h>
#include <GateMutex.h>
#include <OsalSemaphore.h>
#include <Memory.h>

/* Module level headers */
#include <RcmTypes.h>
#include <RcmServer.h>

/* RCM client defines */
#define RCMSERVER_MAX_TABLES 8
#define WAIT_FOREVER 0xFFFFFFFF
#define WAIT_NONE 0x0
#define MAX_NAME_LEN 32
#define RCMSERVER_DESC_TYPE_MASK 0x0F00 /* field mask */
#define RCMSERVER_DESC_TYPE_SHIFT 8 /* field shift width */

#define _RCM_KeyResetValue 0x07FF /* key reset value */
#define _RCM_KeyMask 0x7FF00000 /* key mask in function index */
#define _Rcm_KeyShift 20 /* key bit position in function index */

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/* function table element */
typedef struct RcmServer_FxnTabElem_tag {
    String                      name;
    RcmServer_MsgFxn            addr;
    UInt16                      key;
} RcmServer_FxnTabElem;

typedef struct RcmServer_FxnDesc_tag {
    String name;
    RcmServer_MsgFxn addr;
} RcmServer_FxnDesc;

/*
 * RCM Server instance object structure
 */
typedef struct RcmServer_Object_tag {
    MessageQ_Handle msgQ; /* inbound message queue */
    OsalSemaphore_Handle run; /* synch for starting RCM server thread */
    pthread_t thread; /* server thread object */
    RcmServer_FxnTabElem *fxnTab[RCMSERVER_MAX_TABLES]; /* function table base pointers */
    Int priority;    /* Priority of RCM server */
    Int numAttachedClients; /* number of attached clients */
    Int shutdown; /* signal shutdown by application */
    UInt16 key; /* function index key */
} RcmServer_Object;

/* structure for RcmServer module state */
typedef struct RcmServer_ModuleObject_tag {
    UInt32 setupRefCount;
    /* Ref. count for no. of times setup/destroy was called in this process */
    RcmServer_Config defaultCfg; /* Default config values */
    RcmServer_Params defaultInst_params;
    /* Default instance creation parameters */
} RcmServer_ModuleObject;

/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*
 *  @var    RcmServer_state
 *
 *  @brief  RcmServer state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
RcmServer_ModuleObject RcmServer_module_obj =
{
    .defaultCfg.maxNameLen = MAX_NAME_LEN,
    .defaultCfg.maxTables = RCMSERVER_MAX_TABLES,
    .defaultInst_params.priority = RCMSERVER_REGULAR_PRIORITY,
    .setupRefCount = 0
};

/*
 *  @var    RcmServer_module
 *
 *  @brief  Pointer to RcmServer_module_obj .
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
RcmServer_ModuleObject* RcmServer_module = &RcmServer_module_obj;

/* =============================================================================
 *  private functions
 * =============================================================================
 */
static Int execMsg(RcmServer_Object *obj, RcmClient_Message *msg);

static Int rcmGetFxnAddr(RcmServer_Object *obj, UInt32 fxnIdx,
                    RcmServer_MsgFxn *addrPtr);

static Int rcmGetSymbolIndex(RcmServer_Object *obj, String name, UInt32 *index);
static Void setStatusCode(RcmClient_Packet *packet, UInt16 code);

static UInt16 rcmGetNextKey(RcmServer_Object *obj);

static Void RcmServer_serverRunFxn(IArg arg);

/*
 *  ======== RcmServer_init ========
 * Purpose:
 * Setup RCM server module
 */
Void RcmServer_init (Void)
{
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_init");

    /* TBD: Protect from multiple threads. */
    RcmServer_module->setupRefCount++;

    /* This is needed at runtime so should not be in
     * SYSLINK_BUILD_OPTIMIZE.
     */
    if (RcmServer_module->setupRefCount > 1) {
        /* @retval RcmClient_S_ALREADYSETUP Success: RcmClient module has been
         *          already setup in this process
         */
        status = RcmServer_S_ALREADYSETUP;
        GT_1trace (curTrace,
                GT_1CLASS,
                "RcmServer module has been already setup in this process.\n"
                " RefCount: [%d]\n",
                RcmClient_module->setupRefCount);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmServer_init", status);
}

/*
 *  ======== RcmServer_exit ========
 * Purpose:
 * Clean up RCM Server module
 */
Void RcmServer_exit (Void)
{
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_exit");

    /* TBD: Protect from multiple threads. */
    RcmServer_module->setupRefCount--;

    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (RcmServer_module->setupRefCount < 0) {
        /* @retval RcmServer_S_ALREADYCLEANEDUP :RcmServer module has been
         *          already cleaned up in this process
         */
        status = RcmServer_S_ALREADYCLEANEDUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "RcmServer module has been already been cleaned up in this"
                   " process.\n    RefCount: [%d]\n",
                   RcmServer_module->setupRefCount);
    }

    GT_1trace (curTrace, GT_LEAVE, "RcmServer_exit", status);
}

/*
 *  ======== RcmServer_create ========
 * Purpose:
 * Create a RCM Server instance
 */
Int RcmServer_create(String name,
             RcmServer_Params *params,
             RcmServer_Handle *rcmserverHandle)
{
    MessageQ_Params msgQParams;
    Int i;
    RcmServer_Object *handle = NULL;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_create");

    if (RcmServer_module->setupRefCount == 0) {
        /* Modules is invalidstate*/
        status = RcmServer_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_create",
                             status,
                             "Modules is invalid state!");
        goto exit;
    }

    if (NULL == params) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_create: invalid argument\n");
        status = RcmServer_E_INVALIDARG;
        goto exit;
    }

    if (strlen(name) > RcmServer_module->defaultCfg.maxNameLen) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_create: server name too long\n");
        status = RcmServer_E_FAIL;
        goto exit;
    }

    handle = (RcmServer_Object *)Memory_calloc(NULL,
                         sizeof(RcmServer_Object),
                         0);
    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_create: memory calloc failed\n");
        status = RcmServer_E_NOMEMORY;
        goto exit;
    }

    /* initialize instance state */
    handle->shutdown = FALSE;
    handle->key = 0;
    handle->run = NULL;
    handle->msgQ = NULL;

    /* initialize the function table */
    for (i = 0; i < RcmServer_module->defaultCfg.maxTables; i++) {
        handle->fxnTab[i] = NULL;
    }

    /* create the message queue for inbound messages */
    MessageQ_Params_init(&msgQParams);

    /* create the message queue for inbound messages */
    handle->msgQ = MessageQ_create(name, &msgQParams);
    if (handle->msgQ == NULL) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_create",
                     status,
                     "Unable to create MessageQ");
        status = RcmServer_E_MSGQCREATEFAILED;
        goto exit;
    }

    /* create the run synchronizer */

    handle->run = OsalSemaphore_create(OsalSemaphore_Type_Binary, 0);
    if (handle->run == NULL) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_create",
                     status,
                     "Unable to create sync for RCM server thread");
        status = RcmServer_E_FAIL;
        goto exit;
    }

    handle->numAttachedClients = 0;

    /* create the server thread */
    pthread_create(&(handle->thread), NULL, (void *)&RcmServer_serverRunFxn,
                        (void *)handle);

    *rcmserverHandle = (RcmServer_Handle)handle;

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_create", status);
    return status;
}


/*
 *  ======== RcmServer_Instance_finalize ========
 * Purpose:
 * Delete a RCM Server instance
 */
Int RcmServer_delete(RcmServer_Handle *handlePtr)
{
    Int status = RcmServer_S_SUCCESS;
    Int retval = 0;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_delete");

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmServer_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_delete",
                             status,
                             "Modules is invalid state!");
        goto exit;
    }

    if ((NULL == handlePtr) || (NULL == *handlePtr)) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_delete: invalid argument\n");
        status = RcmServer_E_INVALIDARG;
        goto exit;
    }

    /* Set the shutdown flag */
    (*handlePtr)->shutdown = 1;

    if ((*handlePtr)->thread) {
        /* wait for RCM server thread to exit */
        pthread_join((*handlePtr)->thread, NULL);
    }

    retval = OsalSemaphore_delete(&((*handlePtr)->run));
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     retval,
                     "Unable to delete RCM server thread sync");
        status = RcmServer_E_FAIL;
        goto exit;
    }

    if ((*handlePtr)->msgQ) {
        retval = MessageQ_delete(&((*handlePtr)->msgQ));
        if (retval < 0)
    {
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "RcmServer_delete: Error in MessageQ_delete\n");
        status = RcmServer_E_FAIL;
            goto exit;
        }
    }

    Memory_free(NULL, (RcmServer_Object *)*handlePtr, sizeof(RcmServer_Object));
    *handlePtr = NULL;

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_delete", status);
    return status;
}

/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         RcmServer_create
 */
Int RcmServer_Params_init(RcmServer_Params *params)
{
    Int status = RcmServer_S_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "RcmServer_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (RcmServer_module->setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_Params_init",
                             RcmServer_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = RcmServer_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_Params_init",
                             status,
                             "Argument of type (RcmServer_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        params->priority = RcmServer_module->defaultInst_params.priority;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_Params_init", status);
    return status;
}

/*
 *  ======== RcmServer_addSymbol ========
 * Purpose:
 * Adds symbol to server, return the function index
 */
Int RcmServer_addSymbol(RcmServer_Handle handle,
               String name,
               RcmServer_MsgFxn address,
               UInt32 *fxnIdx)
{
    UInt i, j;
    UInt tabCount;
    Int tabSize;
    RcmServer_FxnTabElem *slot = NULL;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_addSymbol");

    *fxnIdx = 0xFFFFFFFF;

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmServer_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_addSymbol",
                             status,
                             "Modules is invalid state!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_addSymbol: invalid argument\n");
        status = RcmServer_E_INVALIDARG;
        goto exit;
    }

    /* TODO add gate enter/exit */

    /* look for an empty slot to use */
    for (i = 1; i < RcmServer_module->defaultCfg.maxTables; i++) {
        if (handle->fxnTab[i] != NULL) {
            for (j = 0; j < (1 << (i + 4)); j++) {
                if (((handle->fxnTab[i]) + j)->addr == 0) {
                    slot = (handle->fxnTab[i])+j;
                    break;
                }
            }
        }
        else {
            /* all previous tables are full, allocate a new table */
            tabCount = (1 << (i + 4));
            tabSize = tabCount * sizeof(RcmServer_FxnTabElem);
            handle->fxnTab[i] = (RcmServer_FxnTabElem *)Memory_alloc(
                NULL, tabSize, sizeof(Ptr));
            if (NULL == handle->fxnTab[i]) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Memory_alloc",
                             status,
                             "Memory_alloc for"
                            "fxntab failed");
                status = RcmServer_E_NOMEMORY;
                goto exit;
            }

            /* initialize the new table */
            for (j = 0; j < tabCount; j++) {
                ((handle->fxnTab[i])+j)->addr = 0;
                ((handle->fxnTab[i])+j)->name = NULL;
                ((handle->fxnTab[i])+j)->key = 0;
            }

            /* use first slot in new table */
            j = 0;
            slot = (handle->fxnTab[i])+j;
        }

        if (NULL != slot) {
            break;
        }
    }

    /* insert new symbol into given slot */
    if (slot != NULL) {
        slot->addr = address;
        slot->name = (String)Memory_alloc(NULL,
                          strlen(name) + 1,
                          sizeof(Char *));
        if (NULL == slot->name) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "Memory_alloc",
                         status,
                         "Memory_alloc for fxntab failed");
            status = RcmServer_E_NOMEMORY;
            goto exit;
        }
        strcpy(slot->name, name);
        slot->key = rcmGetNextKey(handle);
        *fxnIdx = (slot->key << _Rcm_KeyShift) | (i << 12) | j;
    }
    else {
        /* no more room to add new symbol */
        GT_0trace(curTrace,
              GT_4CLASS,
              "rcmserver_add_symbol: no room to add new symbol");
        status = RcmServer_E_SYMBOLTABLEFULL;
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_addSymbol", status);
    return status;
}


/*
 *  ======== RcmServer_removeSymbol ========
 * Purpose:
 * Removes symbol from server.
 */
Int RcmServer_removeSymbol(RcmServer_Handle handle, String name)
{
    UInt32 fxnIdx;
    UInt tabIdx, tabOff;
    RcmServer_FxnTabElem *slot;
    Int retval;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_removeSymbol");

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RcmServer_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_removeSymbol",
                             status,
                             "Modules is invalid state!");
        goto exit;
    }

    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_removeSymbol: invalid argument\n");
        status = RcmServer_E_INVALIDARG;
        goto exit;
    }

    /* find the symbol in the table */
    retval = rcmGetSymbolIndex(handle, name, &fxnIdx);

    if (retval < 0) {
        status = retval;
        goto exit;
    }

    /* static symbols have bit-31 set, cannot remove these symbols */
    if (fxnIdx & 0x80000000) {
        GT_1trace (curTrace,
            GT_3CLASS,
            "RcmServer_removeSymbol: cannot remove static symbol %s",
            name);
        status = RcmServer_E_SYMBOLSTATIC;
        goto exit;
    }

    /* get slot pointer */
    tabIdx = (fxnIdx & 0xF000) >> 12;
    tabOff = (fxnIdx & 0xFFF);
    slot = (handle->fxnTab[tabIdx]) + tabOff;

    /* clear the table index */
    slot->addr = 0;
    if (slot->name != NULL) {
        Memory_free(NULL, slot->name, strlen(slot->name) + 1);
        slot->name = NULL;
    }
    slot->key = 0;

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_removeSymbol", status);
    return status;
}

/*
 *  ======== RcmServer_start ========
 * Purpose:
 * Starts RCM server thread by posting sync.
 */
Void RcmServer_start(RcmServer_Handle handle)
{
    Int retval = 0;

    /* signal the run synchronizer, unblocks the server thread */
    retval = OsalSemaphore_post(handle->run);
#if !defined(__linux)
    retval = OsalSemaphore_post(handle->run); /* signal once more for Android */
#endif
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             RcmServer_E_INVALIDSTATE,
                             "Could not post sync semaphore");
    }
}

/*
 *  ======== RcmServer_serverRunFxn ========
 * Purpose:
 * RCM server thread function.
 */
Void RcmServer_serverRunFxn(IArg arg)
{
    RcmClient_Packet *packet;
    MessageQ_Msg msgqMsg;
    String name;
    UInt32 fxnIdx;
    RcmServer_MsgFxn fxn;
    RcmClient_Message *rcmMsg;
    UInt16 messageType;
    RcmServer_Handle handle = (RcmServer_Object *)arg;
    Bool running = TRUE;
    Int retval = 0;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_serverRunFxn");

    /* wait until ready to run */
    retval = OsalSemaphore_pend(handle->run, OSALSEMAPHORE_WAIT_FOREVER);
    if (retval < 0) {
        status = RcmServer_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_pend",
                             status,
                             "Could not pend on sync semaphore");
        goto exit;
    }

    /* main processing loop */
    while (running) {

        /* block until message available, skip null messages */
        do {
            retval = MessageQ_get(handle->msgQ, &msgqMsg, 1000 /*MESSAGEQ_FOREVER*/);
            if ((retval < 0) && (retval != MessageQ_E_TIMEOUT)) {
                GT_setFailureReason (curTrace,
                                GT_4CLASS,
                                "MessageQ_get",
                                retval,
                                "RcmClient_module->MessageQ get fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
        } while ((NULL == msgqMsg) && !(handle->shutdown));

        if (handle->shutdown) {
            running = false;

            if (NULL == msgqMsg) {
                continue;
            }
        }

        packet = (RcmClient_Packet *)msgqMsg;
        rcmMsg = &packet->message;

        /* extract the message type from the packet descriptor field */
        messageType = (RCMSERVER_DESC_TYPE_MASK & packet->desc) >>
                RCMSERVER_DESC_TYPE_SHIFT;

        /* process the given message */
        switch (messageType) {

        case RcmClient_Desc_RCM_MSG:
            retval = execMsg(handle, rcmMsg);
            if (retval < 0) {
                switch (retval) {
                    case RcmServer_E_INVALIDFXNIDX:
                        setStatusCode(packet, RcmServer_Status_INVALID_FXN);
                        break;
                    default:
                        setStatusCode(packet, RcmServer_Status_ERROR);
                        break;
                }
            }
            else if (rcmMsg->result < 0) {
                setStatusCode(packet, RcmServer_Status_MSG_FXN_ERR);
            }
            else {
                setStatusCode(packet, RcmServer_Status_SUCCESS);
            }

            retval = MessageQ_put(MessageQ_getReplyQueue
                           (msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
            break;

        case RcmClient_Desc_DPC:
            retval = rcmGetFxnAddr(handle, rcmMsg->fxnIdx, &fxn);

            if (retval < 0)
                setStatusCode(packet, RcmServer_Status_SYMBOL_NOT_FOUND);
            /* TODO: copy the context into a buffer */

            retval = MessageQ_put(MessageQ_getReplyQueue
                               (msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
            /* invoke the function with a null context */
            (*fxn)(0, NULL);
            break;

        case RcmClient_Desc_SYM_ADD:
            break;

        case RcmClient_Desc_SYM_IDX:
            name = (String)rcmMsg->data;
            retval = rcmGetSymbolIndex(handle, name, &fxnIdx);

            if (retval < 0) {
                setStatusCode(packet, RcmServer_Status_SYMBOL_NOT_FOUND);
            }
            else {
                setStatusCode(packet, RcmServer_Status_SUCCESS);
                rcmMsg->data[0] = fxnIdx;
                rcmMsg->result = 0;
            }

            retval = MessageQ_put(MessageQ_getReplyQueue(msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
            break;

        case RcmClient_Desc_CONNECT:

            /* Increase reference count */
           (handle->numAttachedClients)++;

            /* send return message to client */
            retval = MessageQ_put(MessageQ_getReplyQueue(msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
            break;

        case RcmClient_Desc_RCM_NO_REPLY:
            execMsg(handle, rcmMsg);
            retval = MessageQ_free(msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_free",
                             retval,
                             "Msg free fails");
                status = RcmServer_E_FAIL;
                goto exit;
            }
            break;

        default:
            /* TODO */
            break;
        }
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_serverRunFxn", status);
}

/*
 *  ======== rcmGetFxnAddr ========
 * Purpose:
 * Get function addresss using input function index.
 */
Int rcmGetFxnAddr(RcmServer_Handle handle, UInt32 fxnIdx,
    RcmServer_MsgFxn *addrPtr)
{
    UInt i, j;
    UInt16 key;
    RcmServer_FxnTabElem *slot;
    RcmServer_MsgFxn addr = 0;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "rcmGetFxnAddr");

    /* extract the key from the function index */
    key = (fxnIdx & _RCM_KeyMask) >> _Rcm_KeyShift;

    i = (fxnIdx & 0xF000) >> 12;
    if ((i > 0) && (i < RCMSERVER_MAX_TABLES) && (handle->fxnTab[i] != NULL)) {
        /* fetch the function address from the table */
        j = (fxnIdx & 0x0FFF);
        slot = (handle->fxnTab[i])+j;
        addr = slot->addr;

        /* validate the key */
        if (key != slot->key) {
            GT_3trace(curTrace,
                  GT_3CLASS,
                  "rcmGetFxnAddr: key %d does not match entry key %d, fxnIdx %d\n",
                  key, slot->key, fxnIdx);
            status = RcmServer_E_INVALIDFXNIDX;
        }
    }
    if (NULL == addr) {
        status = RcmServer_E_INVALIDFXNIDX;
    }
    if (status >= 0) {
        *addrPtr = addr;
    }

    GT_0trace (curTrace, GT_LEAVE, "rcmGetFxnAddr");
    return status;
}

/*
 *  ======== rcmGetSymbolIndex ========
 * Purpose:
 * Gets the sumbol index given the name of the symbol.
 */
Int rcmGetSymbolIndex(RcmServer_Handle handle, String name, UInt32 *index)
{
    UInt i, j;
    RcmServer_FxnTabElem *slot;
    UInt32 fxnIdx = 0xFFFFFFFF;
    Int status = RcmServer_S_SUCCESS;

    GT_0trace (curTrace, GT_ENTER, "rcmGetSymbolIndex");

    /* search tables for given function name */
    for (i = 0; i < RcmServer_module->defaultCfg.maxTables; i++) {
        if (handle->fxnTab[i] != NULL) {
            for (j = 0; j < (1 << (i + 4)); j++) {
                slot = (handle->fxnTab[i]) + j;
                if (((((handle->fxnTab[i]) + j)->name) != NULL) &&
                    strcmp(((handle->fxnTab[i]) + j)->name, name) == 0) {
                    if (i == 0) {
                        fxnIdx = 0x80000000 | j;
                    } else {
                        fxnIdx = (slot->key << _Rcm_KeyShift) | (i << 12) | j;
                    }
                    break;
                }
            }
        }

        if (0xFFFFFFFF != fxnIdx) {
            break;
        }
    }

    /* raise an error if the symbol was not found */
    if (0xFFFFFFFF == fxnIdx) {
        GT_0trace(curTrace,
              GT_3CLASS,
              "rcmGetSymbolIndex: symbol was not found\n");
        status = RcmServer_E_SYMBOLNOTFOUND;
    }

    if (status >= 0)
        *index = fxnIdx;

    GT_0trace (curTrace, GT_LEAVE, "rcmGetSymbolIndex");
    return status;
}

/*
 *  ======== execMsg ========
 * Purpose:
 * Execute function in address.
 */
Int execMsg(RcmServer_Handle handle, RcmClient_Message *msg)
{
    RcmServer_MsgFxn fxn;
    Int retval;

    GT_0trace (curTrace, GT_ENTER, "execMsg");

    retval = rcmGetFxnAddr(handle, msg->fxnIdx, &fxn);
    if (retval < 0)
        goto leave;

    msg->result = (*fxn)(msg->dataSize, msg->data);

    GT_0trace (curTrace, GT_LEAVE, "execMsg");
leave:
    return retval;
}


/*
 *  ======== rcmGetNextKey ========
 */
UInt16 rcmGetNextKey(RcmServer_Object *obj)
{
    UInt16 key;

    /* TODO: enter gate */

    if (obj->key <= 1) {
        obj->key = _RCM_KeyResetValue;  /* don't use 0 as a key value */
    }
    else {
        (obj->key)--;
    }
    key = obj->key;

    /* TODO: leave gate */

    return(key);
}

/*
 *  ======== setStatusCode ========
 */
Void setStatusCode(RcmClient_Packet *packet, UInt16 code)
{

    /* code must be 0 - 15, it has to fit in 4-bit field */
    if (code >= 16) {
        GT_1trace (curTrace, GT_4CLASS, "setStatusCode: invalid "
            "code %d\n", code);
        return;
    }

    packet->desc &= ~(RCMSERVER_DESC_TYPE_MASK);
    packet->desc |= ((code << RCMSERVER_DESC_TYPE_SHIFT)
        & RCMSERVER_DESC_TYPE_MASK);
}
