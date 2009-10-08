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
 * rcmserver.c
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
#include <MessageQ.h>
#include <GateMutex.h>
#include <OsalSemaphore.h>
#include <Memory.h>

/* Module level headers */
#include <RcmServer.h>

/* RCM client defines */
#define RCMSERVER_MAX_TABLES 8
#define WAIT_FOREVER 0xFFFFFFFF
#define WAIT_NONE 0x0
#define MAX_NAME_LEN 32
#define RCMSERVER_DESC_TYPE_MASK 0x0F00 /* field mask */
#define RCMSERVER_DESC_TYPE_SHIFT 8 /* field shift width */

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/*
 * RCM Server packet structure
 */
typedef struct RcmServer_Packet_tag{
    MessageQ_MsgHeader msgqHeader; /* MessageQ header */
    UInt16 desc; /* protocol version, descriptor, status */
    UInt16 msgId; /* message id */
    RcmServer_Message message; /* client message body (4 words) */
} RcmServer_Packet;

typedef struct FxnDesc_tag {
    String name;
    RcmServer_RemoteFuncPtr addr;
} FxnDesc;

/*
 * RCM Server instance object structure
 */
typedef struct RcmServer_Object_tag {
    MessageQ_Handle msgQ; /* inbound message queue */
    OsalSemaphore_Handle run; /* synch for starting RCM server thread */
    pthread_t thread; /* server thread object */
    FxnDesc *fxnTab[RCMSERVER_MAX_TABLES]; /* function table base pointers */
    Int priority;    /* Priority of RCM server */
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
static Void execMsg(RcmServer_Handle handle, RcmServer_Message *msg);
static RcmServer_RemoteFuncPtr getFxnAddr(RcmServer_Handle handle,
                                          UInt16 fxnIdx);
static UInt16 getSymbolIndex(RcmServer_Handle handle, String name);
Void *RcmServer_serverRunFxn(IArg arg);

/*  @brief      Function to get default configuration for the RcmServer module.
 *
 *  @param      cfgParams   Configuration values.
 *
 *  @sa         RcmServer_setup
 */
Int RcmServer_getConfig (RcmServer_Config *cfgParams)
{
    Int status = RCMSERVER_SOK;

    GT_1trace (curTrace, GT_ENTER, "RcmServer_getConfig", cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        /* @retval RcmServer_EINVALIDARG Argument of type
         *          (RcmServer_Config *) passed is null
         */
        status = RCMSERVER_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_getConfig",
                             status,
                             "Argument of type (RcmServer_Config *) passed"
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        *cfgParams = RcmServer_module->defaultCfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_ENTER, "RcmServer_getConfig", status);

    /* @retval RcmServer_SOK Operation was Successful */
    return status;
}

/*
 *  @brief     Function to setup the RcmServer module.
 *
 *  @param     config  Module configuration parameters
 *
 *  @sa        RcmServerf_destroy
 */
Int RcmServer_setup(const RcmServer_Config *config)
{
    Int status = RCMSERVER_SOK;

    GT_1trace (curTrace, GT_ENTER, "RcmServer_setup", config);

    GT_assert (curTrace, (config != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (config == NULL) {
        /* @retval RcmServer_EINVALIDARG
         *          Argument of type
         *          (RcmServer_Config *) passed is null
         */
        status = RCMSERVER_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_setup",
                             status,
                             "Argument of type (Heap_Config *) passed "
                             "is null!");
    } else if (config->maxNameLen == 0) {
        /* @retval RcmServer_EINVALIDARG Config->defaultCfg.maxNameLen
         *          passed is 0
         */
        status = RCMSERVER_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_setup",
                             status,
                             "Config->defaultCfg.maxNameLen passed is 0!");
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /* TBD: Protect from multiple threads. */
    RcmServer_module->setupRefCount++;

    /* This is needed at runtime so should not be in
     * SYSLINK_BUILD_OPTIMIZE.
     */
    if (RcmServer_module->setupRefCount > 1) {
    /* @retval RCMSERVER_SALREADYSETUP Success: RcmServer module has been
     *          already setup in this process
     */
    status = RCMSERVER_SALREADYSETUP;
    GT_1trace (curTrace,
            GT_1CLASS,
            "RcmServer module has been already setup in this process.\n"
            " RefCount: [%d]\n",
            RcmServer_module->setupRefCount);
    }

    /* @retval RCMSERVER_SOK Operation Successful */
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_setup", status);
    return status;
}

/*
 *  @brief      Function to destroy the RcmServer module.
 *
 *  @param      None
 *
 *  @sa         RcmServer_create
 */
Int RcmServer_destroy(void)
{
    Int status = RCMSERVER_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_destroy");

    /* TBD: Protect from multiple threads. */
    RcmServer_module->setupRefCount--;

    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (RcmServer_module->setupRefCount < 0) {
        /* @retval RCMSERVER_SALREADYSETUP :RcmServer module has been
         *          already setup in this process
         */
        status = RCMSERVER_SALREADYCLEANEDUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "RcmServer module has been already been cleaned up in this"
                   " process.\n    RefCount: [%d]\n",
                   RcmServer_module->setupRefCount);
    }

    /* @retval RCMSERVER_SOK Operation Successful */
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_destroy", status);
    return status;
}

/*
 *  ======== RcmServer_create ========
 * Purpose:
 * Create a RCM Server instance
 */
Int RcmServer_create(String name,
             const RcmServer_Params *params,
             RcmServer_Handle *rcmserverHandle)
{
    MessageQ_Params msgQParams;
    Int i;
    RcmServer_Object *handle = NULL;
    Int status = RCMSERVER_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_create");

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMSERVER_EINVALIDSTATE;
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
        status = RCMSERVER_EINVALIDARG;
        goto exit;
    }

    if (strlen(name) > RcmServer_module->defaultCfg.maxNameLen) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_create: server name too long\n");
        status = RCMSERVER_ENAMELENGTHLIMIT;
        goto exit;
    }

    handle = (RcmServer_Object *)Memory_calloc(NULL,
                         sizeof(RcmServer_Object),
                         0);
    if (NULL == handle) {
        GT_0trace(curTrace,
              GT_4CLASS,
              "RcmServer_create: memory calloc failed\n");
        status = RCMSERVER_EMEMORY;
        goto exit;
    }

    /* initialize instance state */
    handle->msgQ = NULL;

    /* initialize the function table */
    for (i = 0; i < RcmServer_module->defaultCfg.maxTables; i++) {
        handle->fxnTab[i] = NULL;
    }

    /* create the message queue for inbound messages */
    MessageQ_Params_init(NULL, &msgQParams);

    /* create the message queue for inbound messages */
    handle->msgQ = MessageQ_create(name, &msgQParams);
    if (handle->msgQ == NULL) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "MessageQ_create",
                     status,
                     "Unable to create MessageQ");
        status = RCMSERVER_EOBJECT;
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
        status = RCMSERVER_EOBJECT;
        goto exit;
    }

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
    Int status = RCMSERVER_SOK;
    Int retval = 0;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_delete");

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMSERVER_EINVALIDSTATE;
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
        status = RCMSERVER_EHANDLE;
        goto exit;
    }

    if ((*handlePtr)->thread) {
        retval = pthread_cancel((*handlePtr)->thread);
        if (retval < 0)
        {
            GT_0trace (curTrace,
                       GT_4CLASS,
                       "RcmServer_delete: Error in RCM server cancel\n");
            status = RCMSERVER_ECLEANUP;
            goto exit;
        }
        pthread_join((*handlePtr)->thread, NULL);
    }

    retval = OsalSemaphore_delete(&((*handlePtr)->run));
    if (retval < 0) {
        GT_setFailureReason (curTrace,
                     GT_4CLASS,
                     "OsalSemaphore_delete",
                     retval,
                     "Unable to delete RCM server thread sync");
        status = RCMSERVER_ECLEANUP;
        goto exit;
    }

    if ((*handlePtr)->msgQ) {
        retval = MessageQ_delete(&((*handlePtr)->msgQ));
        if (retval < 0)
    {
        GT_0trace (curTrace,
                   GT_4CLASS,
                   "RcmServer_delete: Error in MessageQ_delete\n");
        status = RCMSERVER_ECLEANUP;
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
Int RcmServer_Params_init(RcmServer_Handle handle,
                          RcmServer_Params *params)
{
    Int status = RCMSERVER_SOK;

    GT_1trace (curTrace, GT_ENTER, "RcmServer_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (RcmServer_module->setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_Params_init",
                             RCMSERVER_EINVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = RCMSERVER_EINVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "RcmServer_Params_init",
                             status,
                             "Argument of type (RcmServer_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle == NULL) {
            *params = RcmServer_module->defaultInst_params;
        } else {
            params->priority = handle->priority;
        }
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
               String funcName,
               RcmServer_RemoteFuncPtr address,
               UInt32 *fxnIdx)
{
    UInt i, j;
    UInt tabCount;
    Int tabSize;
    FxnDesc *slot = NULL;
    Int status = RCMSERVER_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_addSymbol");

    *fxnIdx = 0xFFFFFFFF;

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMSERVER_EINVALIDSTATE;
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
        status = RCMSERVER_EHANDLE;
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
            tabCount = (1 << (i + 4));
            tabSize = tabCount * sizeof(FxnDesc);
            handle->fxnTab[i] = (FxnDesc *)Memory_alloc(
                NULL, tabSize, sizeof(Ptr));
            if (NULL == handle->fxnTab[i]) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Memory_alloc",
                             status,
                             "Memory_alloc for"
                            "fxntab failed");
                status = RCMSERVER_EMEMORY;
                goto exit;
            }

            for (j = 0; j < tabCount; j++) {
                ((handle->fxnTab[i])+j)->addr = 0;
                ((handle->fxnTab[i])+j)->name = NULL;
            }

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
                          strlen(funcName) + 1,
                          sizeof(Char *));
        if (NULL == slot->name) {
            GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "Memory_alloc",
                         status,
                         "Memory_alloc for fxntab failed");
            status = RCMSERVER_EMEMORY;
            goto exit;
        }
        strcpy(slot->name, funcName);
        *fxnIdx = (i << 12) | j;
    }
    else {
        /* no more room to add new symbol */
        GT_0trace(curTrace,
              GT_4CLASS,
              "rcmserver_add_symbol: no room to add new symbol");
        status = RCMSERVER_ENOFREESLOT;
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
Int RcmServer_removeSymbol(RcmServer_Handle handle, String funcName)
{
    UInt i, j;
    Int status = RCMSERVER_ESYMBOLNOTFOUND;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_removeSymbol");

    if (RcmServer_module->setupRefCount == 0) {
        /*! @retval FRAMEQ_E_INVALIDSTATE Modules is invalidstate*/
        status = RCMSERVER_EINVALIDSTATE;
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
        status = RCMSERVER_EHANDLE;
        goto exit;
    }

    /* search tables for given function name */
    for (i = 0; i < RcmServer_module->defaultCfg.maxTables; i++) {
        if (handle->fxnTab[i] != NULL) {
            for (j = 0; j < (1 << (i + 4)); j++) {
                if (((((handle->fxnTab[i]) + j)->name) != NULL) &&
                    strcmp(((handle->fxnTab[i]) + j)->name, funcName) == 0) {
                    ((handle->fxnTab[i]) + j) ->name = NULL;
                    ((handle->fxnTab[i]) + j) ->addr = NULL;
                    status = RCMSERVER_SOK;
                    break;
                }
            }
        }

        if(status == RCMSERVER_SOK) {
            break;
        }
    }

exit:
    GT_1trace (curTrace, GT_LEAVE, "RcmServer_removeSymbol", status);
    return status;
}

/*
 *  ======== RcmServer_start ========
 * Purpose:
 * Starts RCM server thread by posting sync.
 */
Int RcmServer_start(RcmServer_Handle handle)
{
    Int retval = 0;
    Int status = RCMSERVER_SOK;

    /* signal the run synchronizer, unblocks the server thread */
    retval = OsalSemaphore_post(handle->run);
    if (retval < 0) {
        status = RCMSERVER_EINVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             status,
                             "Could not post sync semaphore");
    }
    return status;
}

/*
 *  ======== RcmServer_serverRunFxn ========
 * Purpose:
 * RCM server thread function.
 */
Void *RcmServer_serverRunFxn(IArg arg)
{
    RcmServer_Packet *packet;
    MessageQ_Msg msgqMsg;
    String name;
    UInt16 fxnIdx;
    RcmServer_RemoteFuncPtr fxn;
    RcmServer_Message *rcmMsg;
    UInt16 messageType;
    RcmServer_Handle handle = (RcmServer_Object *)arg;
    Bool running = TRUE;
    Int retval = 0;
    Int cancelState = 0;
    Int status = RCMSERVER_SOK;

    GT_0trace (curTrace, GT_ENTER, "RcmServer_serverRunFxn");

    /* wait until ready to run */
    retval = OsalSemaphore_pend(handle->run, WAIT_FOREVER);
    if (retval < 0) {
        status = RCMSERVER_EINVALIDSTATE;
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
            retval = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,
                                                &cancelState);
            pthread_testcancel();
            retval = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,
                                                &cancelState);
			retval = MessageQ_get(handle->msgQ, &msgqMsg, WAIT_NONE);
            if ((retval < 0) && (retval != MESSAGEQ_E_TIMEOUT)) {
                GT_setFailureReason (curTrace,
                                GT_4CLASS,
                                "MessageQ_get",
                                retval,
                                "RcmClient_module->MessageQ get fails");
                status = RCMSERVER_EGETMSG;
                goto exit;
            }
        } while (NULL == msgqMsg);

        packet = (RcmServer_Packet *)msgqMsg;
        rcmMsg = &packet->message;

        /* extract the message type from the packet descriptor field */
        messageType = (RCMSERVER_DESC_TYPE_MASK & packet->desc) >>
                RCMSERVER_DESC_TYPE_SHIFT;

        /* process the given message */
        switch (messageType) {

        case RcmClient_Desc_RCM_MSG:
            execMsg(handle, rcmMsg);

            retval = MessageQ_put(MessageQ_getReplyQueue
                           (msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RCMSERVER_ESENDMSG;
                goto exit;
            }
            break;

        case RcmClient_Desc_DPC:
            fxn = getFxnAddr(handle, rcmMsg->fxnIdx);
            /* TODO: copy the context into a buffer */

            retval = MessageQ_put(MessageQ_getReplyQueue
                               (msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RCMSERVER_ESENDMSG;
                goto exit;
            }
            /* invoke the function with a null context */
            (*fxn)(0, NULL);
            break;

        case RcmClient_Desc_SYM_ADD:
            break;

        case RcmClient_Desc_SYM_IDX:
            name = (String)rcmMsg->data;
            fxnIdx = getSymbolIndex(handle, name);
            if (fxnIdx == 0xFFFF) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "getSymbolIndex",
                             status,
                             "Symbol not found");
                goto exit;
            }
            rcmMsg->result = (Int32)fxnIdx;

            retval = MessageQ_put(MessageQ_getReplyQueue(msgqMsg), msgqMsg);
            if (retval < 0) {
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             retval,
                             "Msg put fails");
                status = RCMSERVER_ESENDMSG;
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
    return (Void *)status;
}

/*
 *  ======== getFxnAddr ========
 * Purpose:
 * Get function addresss using input function index.
 */
RcmServer_RemoteFuncPtr getFxnAddr(RcmServer_Handle handle, UInt16 fxnIdx)
{
    UInt i, j;
    FxnDesc *slot;

    GT_0trace (curTrace, GT_ENTER, "getFxnAddr");

    i = (fxnIdx & 0xF000) >> 12;
    j = (fxnIdx & 0x0FFF);
    slot = (handle->fxnTab[i])+j;

    GT_0trace (curTrace, GT_LEAVE, "getFxnAddr");
    return slot->addr;
}

/*
 *  ======== getSymbolIndex ========
 * Purpose:
 * Gets the sumbol index given the name of the symbol.
 */
UInt16 getSymbolIndex(RcmServer_Handle handle, String name)
{
    UInt i, j;
    UInt16 fxnIdx = 0xFFFF;

    GT_0trace (curTrace, GT_ENTER, "getSymbolIndex");

    /* search tables for given function name */
    for (i = 0; i < RcmServer_module->defaultCfg.maxTables; i++) {
        if (handle->fxnTab[i] != NULL) {
            for (j = 0; j < (1 << (i + 4)); j++) {
                if (((((handle->fxnTab[i]) + j)->name) != NULL) &&
                    strcmp(((handle->fxnTab[i]) + j)->name, name) == 0) {
                    fxnIdx = (i << 12) | j;
                    break;
                }
            }
        }

        if (0xFFFF != fxnIdx) {
            break;
        }
    }

    /* raise an error if the symbol was not found */
    if (0xFFFF == fxnIdx) {
        GT_0trace(curTrace,
              GT_3CLASS,
              "getSymbolIndex: symbol was not found\n");
    }

    GT_0trace (curTrace, GT_LEAVE, "getSymbolIndex");
    return fxnIdx;
}

/*
 *  ======== execMsg ========
 * Purpose:
 * Execute function in address.
 */
Void execMsg(RcmServer_Handle handle, RcmServer_Message *msg)
{
    RcmServer_RemoteFuncPtr fxn;

    GT_0trace (curTrace, GT_ENTER, "execMsg");

    fxn = getFxnAddr(handle, msg->fxnIdx);

    msg->result = (*fxn)(msg->dataSize, msg->data);

    GT_0trace (curTrace, GT_LEAVE, "execMsg");

    return;
}
