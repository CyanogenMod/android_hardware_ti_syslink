/*
 *  Copyright 2001-2009 Texas Instruments - http://www.ti.com/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*============================================================================
 *  @file   RcmClientServerTest.c
 *
 *  @brief  OS-specific sample application framework for RCM module
 *  ============================================================================
 */

 /* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>
#include <String.h>

/* IPC headers */
#if defined (SYSLINK_USE_SYSMGR)
#include <SysMgr.h>
#else /* if defined (SYSLINK_USE_SYSMGR) */
#include <UsrUtilsDrv.h>
#include <HeapBuf.h>
#include <MultiProc.h>
#include <GatePeterson.h>
#include <NameServer.h>
#include <SharedRegion.h>
#include <ListMPSharedMemory.h>
#include <HeapBuf.h>
#include <Notify.h>
#include <NotifyDriverShm.h>
#include <NameServerRemoteNotify.h>
#endif /* if defined(SYSLINK_USE_SYSMGR) */
#include <ProcMgr.h>

/* RCM headers */
#include <RcmClient.h>
#include <RcmServer.h>

/* Sample headers */
#include <RcmTest_Config.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          receive events.
 */
#define BASE_DSP2ARM_INTID     26

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          send events.
 */
#define BASE_ARM2DSP_INTID     55

/*!
 *  Definitions for additional application specified HeapBuf.
 */
#define SHAREDMEM2             0x80000000
#define SHAREDMEMSIZE2         0x400000
#define APP_HEAP_SHAREDBUF     0x2000
#define APP_HEAP_HEAPNAME      "ApplicationHeap0"
#define APP_HEAP_BLOCKSIZE     256

/*RCM client test definitions*/
typedef struct {
    Int a;
} RCM_Remote_FxnArgs;

RcmClient_Handle                rcmClientHandle;
pthread_t                       thread_client; /* client thread object */
UInt                            fxnDoubleIdx;
UInt                            fxnExitIdx;
sem_t                           clientThreadWait;

/*RCM server test definitions*/
typedef struct {
    Int a;
} RCM_Remote_FxnDoubleArgs;

RcmServer_Handle                rcmServerHandle;
pthread_t                       thread_server; /* server thread object */
sem_t                           serverThreadSync;
sem_t                           serverThreadWait;

/*Common definitions*/
Char *                          remoteServerName;
NotifyDriverShm_Handle          notifyDrvHandle;
GatePeterson_Handle             gateHandle_client;
GatePeterson_Handle             gateHandle_server;
NameServerRemoteNotify_Handle   nsrnHandle;
HeapBuf_Handle                  heapHandle = NULL;
GatePeterson_Handle             gateHandle_app_heap;
HeapBuf_Handle                  heapHandle_app_heap = NULL;
MessageQTransportShm_Handle     transportShmHandle;
ProcMgr_Handle                  procMgrHandle;
UInt16                          remoteId;

/*
 *  ======== RCM_Remote_fxnDouble ========
 */
Int32 fxnDouble(UInt32 dataSize, UInt32 *data)
{
    RCM_Remote_FxnDoubleArgs *args;
    Int a;
    Int result;

    Osal_printf ("Executing Remote Function RCM_Remote_fxnDouble\n");

    args = (RCM_Remote_FxnDoubleArgs *)data;
    a = args->a;
    result = a * 2;

    return result;
}

/*
 *  ======== RCM_Remote_fxnDouble ========
 */
static
Int32 fxnExit(UInt32 dataSize, UInt32 *data)
{
    Int status = 0;
    Osal_printf ("Executing Remote Function RCM_Remote_fxnExit \n");
    sem_post (&serverThreadSync);
    return status;
}

/*
 *  ======== GetSymbolIndex ========
 */
Int GetSymbolIndex (Void)
{
    Int status = 0;

    Osal_printf ("\nWait until the RCM Server is run on the Ducati side.\n"
        "Press any key to continue ...\n");
    getchar ();

    /* get remote function index */
    Osal_printf ("\nQuerying server for fxnDouble() function index \n");
    status = RcmClient_getSymbolIndex (rcmClientHandle, "fxnDouble",
                                        &fxnDoubleIdx);
    if (status < 0)
        Osal_printf ("Error getting fxnDouble symbol index [0x%x]\n", status);
    else
        Osal_printf ("fxnDouble() symbol index [0x%x]\n", fxnDoubleIdx);

    Osal_printf ("Querying server for fxnExit() function index \n");
    status = RcmClient_getSymbolIndex (rcmClientHandle, "fxnExit", &fxnExitIdx);
    if (status < 0)
        Osal_printf ("Error getting fxnExit symbol index [0x%x]\n", status);
    else
        Osal_printf ("fxnExit() symbol index [0x%x]\n", fxnExitIdx);

    return status;
}

/*
 *  ======== TestExec ========
 */
Int TestExec (Void)
{
    Int status = 0;
    Int loop;
    RcmClient_Message *rcmMsg = NULL;
    UInt rcmMsgSize;
    RCM_Remote_FxnArgs *fxnDoubleArgs;

    Osal_printf ("\nTesting exec API\n");

    /* work loop: exec () */
    for (loop = 1; loop <= LOOP_COUNT; loop++) {

        // allocate a remote command message
        rcmMsgSize = sizeof(RCM_Remote_FxnArgs);

        Osal_printf ("TestExec: calling RcmClient_alloc \n");
        rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
        if (rcmMsg == NULL) {
            Osal_printf ("TestExec: Error allocating RCM message\n");
            goto exit;
        }

        // fill in the remote command message
        rcmMsg->fxnIdx = fxnDoubleIdx;
        fxnDoubleArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
        fxnDoubleArgs->a = loop;

        // execute the remote command message
        Osal_printf ("TestExec: calling RcmClient_exec \n");
        status = RcmClient_exec (rcmClientHandle, rcmMsg);
        if (status < 0) {
            Osal_printf ("TestExec: RcmClient_exec error. \n");
            goto exit;
        }

        fxnDoubleArgs = (RCM_Remote_FxnArgs *)(&(rcmMsg->data));
        Osal_printf ("TestExec: exec (fxnDouble(%d)), result = %d\n",
            fxnDoubleArgs->a, rcmMsg->result);

        // return message to the heap
        Osal_printf ("TestExec: calling RcmClient_free \n");
        RcmClient_free (rcmClientHandle, rcmMsg);
    }

exit:
    Osal_printf ("TestExec: Leaving Testexec ()\n");
    return status;
}

/*
 *  ======== TestExecDpc ========
 */
Int TestExecDpc (Void)
{
    Int status = 0;
    Int loop;
    RcmClient_Message *rcmMsg = NULL;
    UInt rcmMsgSize;
    RCM_Remote_FxnArgs *fxnDoubleArgs;

    Osal_printf ("TestExecDpc: Testing execDpc API\n");

    /* work loop: RcmClient_execDpc () */
    for (loop = 1; loop <= LOOP_COUNT; loop++) {

        // allocate a remote command message
        rcmMsgSize = sizeof(RCM_Remote_FxnArgs);

        Osal_printf ("TestExecDpc: calling RcmClient_alloc \n");
        rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
        if (rcmMsg == NULL) {
            Osal_printf ("TestExecDpc: Error allocating RCM message\n");
            goto exit;
        }

        // fill in the remote command message
        rcmMsg->fxnIdx = fxnDoubleIdx;
        fxnDoubleArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
        fxnDoubleArgs->a = loop;

        // execute the remote command message
        Osal_printf ("TestExecDpc: calling RcmClient_execDpc \n");
        status = RcmClient_execDpc (rcmClientHandle, rcmMsg);
        if (status < 0) {
            Osal_printf ("TestExecDpc: RcmClient_execDpc error. \n");
            goto exit;
        }

        fxnDoubleArgs = (RCM_Remote_FxnArgs *)(&(rcmMsg->data));
        Osal_printf ("TestExecDpc: exec (fxnDouble(%d)), result = %d",
            fxnDoubleArgs->a, rcmMsg->result);

        // return message to the heap
        Osal_printf ("TestExecDpc: calling RcmClient_free \n");
        RcmClient_free (rcmClientHandle, rcmMsg);
    }

exit:
    Osal_printf ("Leaving TestexecDpc ()\n");
    return status;
}

/*
 *  ======== TestExecNoWait ========
 */
Int TestExecNoWait(void)
{
    Int                     status = 0;
    Int                     loop, job;
    unsigned short          msgIdAry[JOB_COUNT];
    RcmClient_Message *     rcmMsg = NULL;
    UInt                    rcmMsgSize;
    RCM_Remote_FxnArgs *    fxnDoubleArgs;

    Osal_printf ("\nTestExecNoWait: Testing TestExecNoWait API\n");

    /* work loop: execNoWait(), exec () */
    for (loop = 1; loop <= LOOP_COUNT; loop++) {
        /*
         * issue process jobs
         */
        for (job = 1; job <= JOB_COUNT; job++) {
            // allocate a remote command message
            rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
            Osal_printf ("TestExecNoWait: calling RcmClient_alloc \n");
            rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
            if (rcmMsg == NULL) {
                Osal_printf ("TestExecNoWait: Error allocating RCM message\n");
                goto exit;
            }
            // fill in the remote command message
            rcmMsg->fxnIdx = fxnDoubleIdx;
            fxnDoubleArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
            fxnDoubleArgs->a = job;

            // execute the remote command message
            Osal_printf ("TestExecNoWait: calling RcmClient_execNoWait \n");
            status = RcmClient_execNoWait (rcmClientHandle, rcmMsg,
                                            &msgIdAry[job-1]);
            if (status < 0) {
                Osal_printf ("TestExecNoWait: RcmClient_execNoWait error. \n");
                goto exit;
            }
        }

        /*
         * reclaim process jobs
         */
        for (job = 1; job <= JOB_COUNT; job++) {
            Osal_printf ("TestExecNoWait: calling RcmClient_waitUntilDone \n");
            status = RcmClient_waitUntilDone (rcmClientHandle, msgIdAry[job-1],
                                                rcmMsg);
            if (status < 0) {
                Osal_printf ("TestExecNoWait: RcmClient_waitUntilDone error\n");
                goto exit;
            }

            Osal_printf ("TestExecNoWait: msgId: %d, result = %d",
                            msgIdAry[job-1], rcmMsg->result);

            // return message to the heap
            Osal_printf ("TestExecNoWait: calling RcmClient_free \n");
            RcmClient_free (rcmClientHandle, rcmMsg);
        }
    }

exit:
    Osal_printf ("TestExecNoWait: Leaving TestExecNoWait()\n");
    return status;
}

/*
 *  ======== ipc_setup ========
 */
Int ipc_setup (Int testCase)
{
    Char *                          procName;
    UInt16                          procId;
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
    UInt32                          shAddrBase2;
    UInt32                          curShAddr;
    UInt32                          nsrnEventNo;
    UInt32                          mqtEventNo;
#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
    UInt32                          entry_point = 0;
    ProcMgr_StartParams             start_params;
#endif
#if defined(SYSLINK_USE_LOADER)
    Char *                          image_name;
    Char                            uProcId;
    UInt32                          fileId;
#endif
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config                   config;
#else
    UInt32                          curShAddr_temp;
    Notify_Config                   notifyConfig;
    NotifyDriverShm_Config          notifyDrvShmConfig;
    NameServerRemoteNotify_Config   nsrConfig;
    SharedRegion_Config             sharedRegConfig;
    ListMPSharedMemory_Config       listMPSharedConfig;
    GatePeterson_Config             gpConfig;
    HeapBuf_Config                  heapbufConfig;
    MessageQTransportShm_Config     msgqTransportConfig;
    MessageQ_Config                 messageqConfig;
    MultiProc_Config                multiProcConfig;
    NotifyDriverShm_Params          notifyShmParams;
    GatePeterson_Params             gateParams;
    NameServerRemoteNotify_Params   nsrParams;
    HeapBuf_Params                  heapbufParams;
    MessageQTransportShm_Params     msgqTransportParams;
#endif /* if defined(SYSLINK_USE_SYSMGR) */
    GatePeterson_Params             gateParams_app_heap;
    HeapBuf_Params                  heapbufParams_app_heap;
    Int                             status = 0;

    Osal_printf ("ipc_setup: Setup IPC componnets \n");

    switch(testCase) {
    case 1:
        Osal_printf ("ipc_setup: RCM test with RCM client and server on "
                    "Sys M3\n\n");
        remoteServerName = SYSM3_SERVER_NAME;
        procName = SYSM3_PROC_NAME;
        nsrnEventNo = NSRN_NOTIFY_EVENTNO;
        mqtEventNo = TRANSPORT_NOTIFY_EVENTNO;
        break;
    case 2:
        Osal_printf ("ipc_setup: RCM test with RCM client and server on "
                    "App M3\n\n");
        remoteServerName = APPM3_SERVER_NAME;
        procName = APPM3_PROC_NAME;
        nsrnEventNo = NSRN_NOTIFY_EVENTNO1;
        mqtEventNo = TRANSPORT_NOTIFY_EVENTNO1;
        break;
    default:
        Osal_printf ("ipc_setup: Please pass valid arg "
                    "(1-Sys M3, 2-App M3)\n\n");
        goto exit;
        break;
    }

    /* Get and set GPP MultiProc ID by name. */
#if !defined(SYSLINK_USE_SYSMGR)
    multiProcConfig.maxProcessors = 4;
    multiProcConfig.id = 0;
    String_cpy (multiProcConfig.nameList [0], "MPU");
    String_cpy (multiProcConfig.nameList [1], "Tesla");
    String_cpy (multiProcConfig.nameList [2], "SysM3");
    String_cpy (multiProcConfig.nameList [3], "AppM3");
    status = MultiProc_setup(&multiProcConfig);
    if (status < 0) {
        Osal_printf ("Error in MultiProc_setup [0x%x]\n", status);
        goto exit;
    }
#endif

    /* size (in bytes) of RCM header including the messageQ header */
    /* RcmClient_Message member data[1] is the start of the payload */
    Osal_printf ("Size of RCM header in bytes = %d \n",
                            RcmClient_getHeaderSize());

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
        goto exit;
    }
#else /* if defined(SYSLINK_USE_SYSMGR) */

    UsrUtilsDrv_setup ();

    /* NameServer and NameServerRemoteNotify module setup */
    status = NameServer_setup ();
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in NameServer_setup [0x%x]\n",
                        status);
        goto exit;
    }
    Osal_printf ("ipc_setup: NameServer_setup Status [0x%x]\n", status);

    NameServerRemoteNotify_getConfig (&nsrConfig);
    status = NameServerRemoteNotify_setup (&nsrConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in NameServerRemoteNotify_setup "
                    "[0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("NameServerRemoteNotify_setup Status [0x%x]\n",
                    status);

    /*
     *  Need to define the shared region. The IPC modules use this
     *  to make portable pointers. All processors need to add this
     *  same call with their base address of the shared memory region.
     *  If the processor cannot access the memory, do not add it.
     */

    /* SharedRegion module setup */
    sharedRegConfig.gateHandle = NULL;
    sharedRegConfig.heapHandle = NULL;
    sharedRegConfig.maxRegions = 4;
    status = SharedRegion_setup(&sharedRegConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in SharedRegion_setup."
                    "Status [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: SharedRegion_setup Status [0x%x]\n",
                        status);

    /* ListMPSharedMemory module setup */
    ListMPSharedMemory_getConfig (&listMPSharedConfig);
    status = ListMPSharedMemory_setup (&listMPSharedConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in ListMPSharedMemory_setup."
                     " Status [0x%x]\n",
                     status);
        goto exit;
    }
    Osal_printf ("ipc_setup: ListMPSharedMemory_setup Status [0x%x]\n",
                     status);

    /* HeapBuf module setup */
    HeapBuf_getConfig (&heapbufConfig);
    status = HeapBuf_setup (&heapbufConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in HeapBuf_setup."
                       "Status [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: HeapBuf_setup Status [0x%x]\n",
                     status);

    /* GatePeterson module setup */
    GatePeterson_getConfig (&gpConfig);
    status = GatePeterson_setup (&gpConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in GatePeterson_setup."
                    "Status [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: GatePeterson_setup Status [0x%x]\n",
                    status);

    /* Setup Notify module and NotifyDriverShm module */
    Notify_getConfig (&notifyConfig);
    notifyConfig.maxDrivers = 2;
    status = Notify_setup (&notifyConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in Notify_setup [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: Notify_setup Status [0x%x]\n", status);

    NotifyDriverShm_getConfig (&notifyDrvShmConfig);
    status = NotifyDriverShm_setup (&notifyDrvShmConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in NotifyDriverShm_setup "
                        "[0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: NotifyDriverShm_setup Status [0x%x]\n",
                      status);

    /* Setup MessageQ module and MessageQTransportShm module */
    MessageQ_getConfig (&messageqConfig);
    status = MessageQ_setup (&messageqConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in MessageQ_setup [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: MessageQ_setup Status [0x%x]\n", status);

    MessageQTransportShm_getConfig (&msgqTransportConfig);
    status = MessageQTransportShm_setup (&msgqTransportConfig);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in MessageQTransportShm_setup "
                       "[0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ipc_setup: MessageQTransportShm_setup Status "
                       "[0x%x]\n", status);

#endif /* if defined(SYSLINK_USE_SYSMGR) */

    procId = MultiProc_getId (SYSM3_PROC_NAME);
    remoteId = MultiProc_getId (procName);

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle, procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_open Status [0x%x]\n", status);

    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr (procMgrHandle,
                                    (Ptr) &shAddrBase,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM,
                                    ProcMgr_AddrType_SlaveVirt);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                       status);
        goto exit;
    }
    Osal_printf ("Virt address of shared address base #1:"
                         " [0x%x]\n", shAddrBase);

    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr (procMgrHandle,
                                    (Ptr) &shAddrBase1,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM1,
                                    ProcMgr_AddrType_SlaveVirt);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                           status);
        goto exit;
    }
    Osal_printf ("Virt address of shared address base #2:"
                            " [0x%x]\n", shAddrBase1);

    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr (procMgrHandle,
                                    (Ptr) &shAddrBase2,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM2,
                                    ProcMgr_AddrType_SlaveVirt);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("Virt address of shared address base #3:"
                    " [0x%x]\n", shAddrBase2);

    curShAddr = shAddrBase;
    /* Add the region to SharedRegion module. */
    status = SharedRegion_add (0,
                               (Ptr) curShAddr,
                               SHAREDMEMSIZE);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in SharedRegion_add [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("SharedRegion_add [0x%x]\n", status);

    /* Add the region to SharedRegion module. */
    status = SharedRegion_add (1,
                               (Ptr) shAddrBase1,
                               SHAREDMEMSIZE1);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in SharedRegion_add1 [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("SharedRegion_add1 [0x%x]\n", status);

    /* Add the region to SharedRegion module. */
    status = SharedRegion_add (2,
                               (Ptr) shAddrBase2,
                               SHAREDMEMSIZE2);
    if (status < 0) {
        Osal_printf ("ipc_setup: Error in SharedRegion_add2 [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("SharedRegion_add2 [0x%x]\n", status);

#if !defined(SYSLINK_USE_SYSMGR)
    /* Create instance of NotifyDriverShm */
    NotifyDriverShm_Params_init (NULL, &notifyShmParams);
    //notifyShmParams.sharedAddr= (UInt32)curShAddr;
    //notifyShmParams.sharedAddrSize = 0x4000;
    /* NotifyDriverShm_sharedMemReq (&notifyShmParams); */
    notifyShmParams.numEvents          = 32;
    notifyShmParams.numReservedEvents  = 0;
    notifyShmParams.sendEventPollCount = (UInt32) -1;
    notifyShmParams.recvIntId          = BASE_DSP2ARM_INTID;
    notifyShmParams.sendIntId          = BASE_ARM2DSP_INTID;
    notifyShmParams.remoteProcId       = procId;

    if (testCase == 1) {
        /* Increment the offset for the next allocation for MPU-SysM3 */
        curShAddr += NOTIFYMEMSIZE;
    }
    else if (testCase == 2) {
        /* Reset the address for the next allocation for MPU-AppM3 */
        curShAddr = shAddrBase1;
    }

    /* Create instance of NotifyDriverShm */
    notifyDrvHandle = NotifyDriverShm_create (
                                              "NOTIFYDRIVER_DUCATI",
                                              &notifyShmParams);
    Osal_printf ("ipc_setup: NotifyDriverShm_create Handle: [0x%x]\n",
                 notifyDrvHandle);
    if (notifyDrvHandle == NULL) {
        Osal_printf ("ipc_setup: Error in NotifyDriverShm_create\n");
        goto exit;
    }

    GatePeterson_Params_init (gateHandle_server, &gateParams);
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
    gateParams.sharedAddr     = (Ptr)(curShAddr);
    Osal_printf ("ipc_setup: Memory required for GatePeterson instance "
                "[0x%x] bytes \n",
                 gateParams.sharedAddrSize);

    gateHandle_server = GatePeterson_create (&gateParams);
    if (gateHandle_server == NULL) {
        Osal_printf ("ipc_setup: Error in GatePeterson_create [0x%x]\n",
                        status);
        goto exit;
    }
    Osal_printf ("ipc_setup: GatePeterson_create Status [0x%x]\n",
                    status);
#endif

#if defined(SYSLINK_USE_LOADER)
    if (testCase == 1)
        image_name = "./RCMSrvClnt_MPUSYS_Test_Core0.xem3";
    else if (testCase == 2)
        image_name = "./Notify_MPUSYS_reroute_Test_Core0.xem3";
    uProcId = MultiProc_getId (SYSM3_PROC_NAME);
    Osal_printf ("SYSM3 Load: loading the SYSM3 image %s\n", image_name);
    Osal_printf ("SYSM3 Load: uProcId = %d\n", uProcId);
    status = ProcMgr_load (procMgrHandle, image_name, 2, &image_name,
                            &entry_point, &fileId, uProcId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_load SysM3 image [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_load SysM3 image Status [0x%x]\n", status);
#endif
#if defined(SYSLINK_USE_SYSMGR) ||(SYSLINK_USE_LOADER)
    start_params.proc_id = MultiProc_getId (SYSM3_PROC_NAME);
    Osal_printf ("SYSM3 Load: start_params.proc_id = %d\n",
                start_params.proc_id);
    status = ProcMgr_start (procMgrHandle, entry_point, &start_params);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_start SysM3 [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_start SysM3 Status [0x%x]\n", status);
#endif

    /*FIXME: */
    if(testCase == 2) {
#if defined(SYSLINK_USE_LOADER)
        image_name = "./RCMSrvClnt_MPUAPP_Test_Core1.xem3";
        uProcId = MultiProc_getId (APPM3_PROC_NAME);
        Osal_printf ("APPM3 Load: loading the APPM3 image %s\n",
                    image_name);
        Osal_printf ("APPM3 Load: uProcId = %d\n", uProcId);
        status = ProcMgr_load (procMgrHandle, image_name, 2, &image_name,
                                &entry_point, &fileId, uProcId);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_load AppM3 image [0x%x]\n", status);
            goto exit;
        }
        Osal_printf("ProcMgr_load AppM3 image Status [0x%x]\n",status);
#endif /* defined(SYSLINK_USE_LOADER) */
#if defined(SYSLINK_USE_SYSMGR) ||(SYSLINK_USE_LOADER)
        start_params.proc_id = MultiProc_getId (APPM3_PROC_NAME);
        Osal_printf("APPM3 Load: start_params.proc_id = %d\n",
                    start_params.proc_id);
        status = ProcMgr_start (procMgrHandle, entry_point, &start_params);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_start AppM3 [0x%x]\n", status);
            goto exit;
        }
        Osal_printf ("ProcMgr_start AppM3 Status [0x%x]\n", status);
#endif
    }

#if !defined(SYSLINK_USE_SYSMGR)
    Osal_printf ("ipc_setup: Please break the platform and load the Ducati "
                "image now. Press any key to continue ...\n");
    getchar ();

    Osal_printf ("ipc_setup: Opening the Gate\n");
    if (testCase == 1) {
        curShAddr_temp = curShAddr + GATEPETERSONMEMSIZE + HEAPBUFMEMSIZE
                            + NSRN_MEMSIZE + TRANSPORTMEMSIZE
                            + MESSAGEQ_NS_MEMSIZE + HEAPBUF_NS_MEMSIZE;
    }
    else
    {
        curShAddr_temp = curShAddr + GATEPETERSONMEMSIZE + HEAPBUFMEMSIZE
                            + NSRN_MEMSIZE + TRANSPORTMEMSIZE
                            + MESSAGEQ_NS_MEMSIZE + HEAPBUF_NS_MEMSIZE
                            + HEAPBUFMEMSIZE1;
    }

    GatePeterson_Params_init (gateHandle_client, &gateParams);
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
    Osal_printf ("ipc_setup: Memory required for GatePeterson instance "
                    "[0x%x] bytes \n",
                    gateParams.sharedAddrSize);

    do {
        gateParams.sharedAddr     = (Ptr)(curShAddr_temp);
        status = GatePeterson_open (&gateHandle_client,
                                    &gateParams);
    }
    while ((status == GATEPETERSON_E_NOTFOUND) ||
            (status == GATEPETERSON_E_VERSION));

    if (status < 0) {
        Osal_printf ("ipc_setup: Error in GatePeterson_open [0x%x]\n",
                        status);
        goto exit;
    }
    Osal_printf ("ipc_setup: GatePeterson_open Status [0x%x]\n",
                        status);

    /* Increment the offset for the next allocation */
    curShAddr += GATEPETERSONMEMSIZE;

    /* Create the heap. */
    HeapBuf_Params_init(NULL, &heapbufParams);
    if (testCase == 1)
        heapbufParams.name       = HEAPNAME_SYSM3;
    else if (testCase == 2)
        heapbufParams.name       = HEAPNAME_APPM3;
    heapbufParams.sharedAddr     = (Ptr)(curShAddr);
    heapbufParams.align          = 128;
    heapbufParams.numBlocks      = 4;
    heapbufParams.blockSize      = MSGSIZE;
    heapbufParams.gate           = (Gate_Handle) gateHandle_client;
    heapbufParams.sharedAddrSize = HeapBuf_sharedMemReq (&heapbufParams,
                                    &heapbufParams.sharedBufSize);
    heapbufParams.sharedBuf      = (Ptr)(curShAddr +
                                    heapbufParams.sharedAddrSize);
    heapHandle = HeapBuf_create (&heapbufParams);
    if (heapHandle == NULL) {
        Osal_printf ("ipc_setup: Error in HeapBuf_create\n");
        goto exit;
    }
    Osal_printf ("ipc_setup: HeapBuf_create Handle [0x%x]\n",
                  heapHandle);

    /* Register this heap with MessageQ */
    if (testCase == 1)
        MessageQ_registerHeap (heapHandle, HEAPID_SYSM3);
    else if (testCase == 2)
        MessageQ_registerHeap (heapHandle, HEAPID_APPM3);
    }

    /* Increment the offset for the next allocation */
    curShAddr += HEAPBUFMEMSIZE;
    /*
     *  Create the NameServerRemote implementation that is used to
     *  communicate with the remote processor. It uses some shared
     *  memory and the Notify module.
     *
     *  Note that this implementation uses Notify to communicate, so
     *  interrupts need to be enabled. On BIOS, that does not occur
     *  until after main returns.
     */
    NameServerRemoteNotify_Params_init (NULL, &nsrParams);
    nsrParams.notifyDriver  = notifyDrvHandle;
    nsrParams.notifyEventNo = nsrnEventNo;
    nsrParams.sharedAddr    = (Ptr)curShAddr;
    nsrParams.gate      = (Ptr)gateHandle_client;
    nsrParams.sharedAddrSize  = NSRN_MEMSIZE;
    nsrnHandle = NameServerRemoteNotify_create (remoteId, &nsrParams);
    if (nsrnHandle == NULL) {
        Osal_printf ("ipc_setup: Error in NotifyDriverShm_create\n");
        goto exit;
    }
    Osal_printf ("ipc_setup: NameServerRemoteNotify_create handle "
                     "[0x%x]\n", nsrnHandle);

    /* Increment the offset for the next allocation */
    curShAddr += NSRN_MEMSIZE;

    MessageQTransportShm_Params_init (NULL, &msgqTransportParams);

    msgqTransportParams.sharedAddr = (Ptr)curShAddr;
    msgqTransportParams.gate = (Gate_Handle) gateHandle_client;
    msgqTransportParams.notifyEventNo = mqtEventNo;
    msgqTransportParams.notifyDriver = notifyDrvHandle;
    msgqTransportParams.sharedAddrSize =
                 MessageQTransportShm_sharedMemReq (&msgqTransportParams);

    transportShmHandle = MessageQTransportShm_create (remoteId,
                                        &msgqTransportParams);
    if (transportShmHandle == NULL) {
        Osal_printf ("ipc_setup: Error in MessageQTransportShm_create\n");
        goto exit;
    }
    Osal_printf ("ipc_setup: MessageQTransportShm_create handle "
                   "[0x%x]\n", transportShmHandle);

#endif

    GatePeterson_Params_init (gateHandle_client, &gateParams_app_heap);
    gateParams_app_heap.sharedAddrSize =
                       GatePeterson_sharedMemReq (&gateParams_app_heap);
    Osal_printf ("ipc_setup: Memory required for GatePeterson instance "
                    "[0x%x] bytes \n",
                    gateParams_app_heap.sharedAddrSize);

    do {
        gateParams_app_heap.sharedAddr     = (Ptr)(shAddrBase2);
        status = GatePeterson_open (&gateHandle_app_heap,
                                    &gateParams_app_heap);
    }
    while ((status == GATEPETERSON_E_NOTFOUND) ||
            (status == GATEPETERSON_E_VERSION));

    if (status < 0) {
        Osal_printf ("ipc_setup: Error in GatePeterson_open [0x%x]\n",
                      status);
        goto exit;
    }
    Osal_printf ("ipc_setup: GatePeterson_open Status [0x%x]\n", status);

    /* Increment the offset for the next allocation */
    curShAddr += GATEPETERSONMEMSIZE;

    /* Create the heap. */
    HeapBuf_Params_init(NULL, &heapbufParams_app_heap);
    heapbufParams_app_heap.name           = APP_HEAP_HEAPNAME;
    heapbufParams_app_heap.sharedAddr     = (Ptr)
                              (shAddrBase2 + GATEPETERSONMEMSIZE);
    heapbufParams_app_heap.align          = 128;
    heapbufParams_app_heap.numBlocks      = 4;
    heapbufParams_app_heap.blockSize      = APP_HEAP_BLOCKSIZE;
    heapbufParams_app_heap.gate           = (Gate_Handle) gateHandle_client;
    heapbufParams_app_heap.sharedAddrSize = HeapBuf_sharedMemReq
          (&heapbufParams_app_heap, &heapbufParams_app_heap.sharedBufSize);
    heapbufParams_app_heap.sharedBuf      = (Ptr)
                      (shAddrBase2 + APP_HEAP_SHAREDBUF);

    status = HeapBuf_open (&heapHandle_app_heap, &heapbufParams_app_heap);
    if(status < 0) {
        Osal_printf ("ipc_setup: Error in HeapBuf_create\n");
        goto exit;
    }
    Osal_printf ("ipc_setup: HeapBuf_create Handle [0x%x]\n",
                            heapHandle_app_heap);

exit:
    Osal_printf ("ipc_setup: Leaving ipc_setup()\n");
    return status;
}

/*
 *  ======== RcmServerThreadFxn ========
 *     RCM server test thread function
*/
Void RcmServerThreadFxn(Void *arg)
{
    RcmServer_Config                cfgParams;
    RcmServer_Params                rcmServer_Params;
    UInt                            fxnIdx;
    Char *                          rcmServerName = RCMSERVER_NAME;
    Int status = 0;

    /* Get default config for rcm client module */
    Osal_printf ("RcmServerThreadFxn: Get default config for rcm server "
                    "module.\n");
    status = RcmServer_getConfig (&cfgParams);
    if (status < 0) {
        Osal_printf ("RcmServerThreadFxn: Error in RCM Server module get "
                    "config \n");
        goto exit;
    }
    Osal_printf ("RcmServerThreadFxn: RCM Client module get config "
                    "passed \n");

    /* rcm client module setup*/
    Osal_printf ("RcmServerThreadFxn: RCM Server module setup.\n");
    status = RcmServer_setup (&cfgParams);
    if (status < 0) {
        Osal_printf ("RcmServerThreadFxn: Error in RCM Server module setup \n");
        goto exit;
    }
    Osal_printf ("RcmServerThreadFxn: RCM Server module setup passed \n");

    /* rcm client module params init*/
    Osal_printf ("RcmServerThreadFxn: rcm client module params init.\n");
    status = RcmServer_Params_init (NULL, &rcmServer_Params);
    if (status < 0) {
        Osal_printf ("RcmServerThreadFxn: Error in RCM Server instance params "
                        "init \n");
        goto exit;
    }
    Osal_printf ("RcmServerThreadFxn: RCM Server instance params init "
                   "passed \n");

    /* create the RcmServer instance */
    Osal_printf ("RcmServerThreadFxn: Creating RcmServer instance.\n");
    status = RcmServer_create (rcmServerName, &rcmServer_Params,
                                &rcmServerHandle);
    if (status < 0) {
        Osal_printf ("RcmServerThreadFxn: Error in RCM Server create.\n");
        goto exit;
    }
    Osal_printf ("RcmServerThreadFxn: RCM Server Create passed \n");

    sem_init (&serverThreadSync, 0, 0);

    /* Register the remote functions */
    Osal_printf ("Registering remote function - 1\n");
    status = RcmServer_addSymbol (rcmServerHandle, "fxnDouble", fxnDouble,
                                    &fxnIdx);
    if ((status < 0) || (fxnIdx == 0xFFFFFFFF)) {
        Osal_printf ("Add symbol failed.\n");
        goto exit;
    }

    Osal_printf ("Registering remote function - 2\n");
    status = RcmServer_addSymbol (rcmServerHandle, "fxnExit", fxnExit, &fxnIdx);
    if ((status < 0) || (fxnIdx == 0xFFFFFFFF)) {
        Osal_printf ("Add symbol failed.\n");
        goto exit;
    }

    Osal_printf ("Start RCM server thread \n");
    status = RcmServer_start (rcmServerHandle);
    if (status < 0) {
        Osal_printf ("Error in RCM Server start.\n");
        goto exit;
    }
    Osal_printf ("RCM Server start passed \n");

    sem_wait (&serverThreadSync);

    sem_post (&serverThreadWait);

exit:
    Osal_printf ("RcmServerThreadFxn: Leaving RCM server test thread "
                    "function \n");
    return;
}

/*
 *  ======== RcmServerThreadFxn ========
 *     RCM server test thread function
*/
Void RcmClientThreadFxn(Void *arg)
{
    RcmClient_Config                cfgParams;
    RcmClient_Params                rcmClient_Params;
    Int                             count = 0;
    Int                             testCase;
    Int                             status = 0;

    /* Get default config for rcm client module */
    Osal_printf ("RcmClientThreadFxn: Get default config for rcm client "
                    "module.\n");
    status = RcmClient_getConfig(&cfgParams);
    if (status < 0) {
        Osal_printf ("RcmClientThreadFxn: Error in RCM Client module get "
                    "config \n");
        goto exit;
    }
    Osal_printf ("RcmClientThreadFxn: RCM Client module get config "
                   "passed \n");
    cfgParams.defaultHeapBlockSize = MSGSIZE;

    /* rcm client module setup*/
    Osal_printf ("RcmClientThreadFxn: RCM Client module setup.\n");
    status = RcmClient_setup(&cfgParams);
    if (status < 0) {
        Osal_printf ("RcmClientThreadFxn: Error in RCM Client module setup \n");
        goto exit;
    }
    Osal_printf ("RcmClientThreadFxn: RCM Client module setup passed \n");

    /* rcm client module params init*/
    Osal_printf ("RcmClientThreadFxn: RCM Client module params init.\n");
    status = RcmClient_Params_init(NULL, &rcmClient_Params);
    if (status < 0) {
        Osal_printf ("RcmClientThreadFxn: Error in RCM Client instance params "
                        "init \n");
        goto exit;
    }
    Osal_printf ("RcmClientThreadFxn: RCM Client instance params init "
                    "passed \n");

    /* create an rcm client instance */
    Osal_printf ("RcmClientThreadFxn: Creating RcmClient instance \n");
    rcmClient_Params.callbackNotification = 0; /* disable asynchronous exec */
    testCase = (Int)arg;
    if (testCase == 1)
        rcmClient_Params.heapId = HEAPID_SYSM3;
    else if (testCase == 2)
        rcmClient_Params.heapId = HEAPID_APPM3;

    while ((rcmClientHandle == NULL) && (count++ < MAX_CREATE_ATTEMPTS)) {
        status = RcmClient_create (remoteServerName, &rcmClient_Params,
                                    &rcmClientHandle);
        if (status < 0) {
            if (status == RCMCLIENT_ESERVER) {
                Osal_printf ("RcmClientThreadFxn: Unable to open remote"
                                "server %d time \n", count);
            }
            else {
                Osal_printf ("RcmClientThreadFxn: Error in RCM Client "
                                "create \n");
                goto exit;
            }
        }
        Osal_printf ("RcmClientThreadFxn: RCM Client create passed \n");
    }

    if (MAX_CREATE_ATTEMPTS <= count) {
        Osal_printf ("RcmClientThreadFxn: Timeout... could not connect with"
                        "remote server\n");
    }

    status = GetSymbolIndex ();
    if (status < 0) {
        Osal_printf ("RcmClientThreadFxn: Error in GetSymbolIndex \n");
        goto exit;
    }

    status = TestExec ();
    if (status < 0) {
        Osal_printf ("RcmClientThreadFxn: Error in TestExec \n");
        goto exit;
    }

    sem_post (&clientThreadWait);

exit:
    Osal_printf ("RcmClientThreadFxn: Leaving RCM client test thread "
                    "function \n");
    return;
}

/*
 *  ======== RcmTestCleanup ========
 */
Int RcmTestCleanup (Int testCase)
{
    Int status = 0;
    RcmClient_Message *rcmMsg = NULL;
    UInt rcmMsgSize;
    RCM_Remote_FxnArgs *fxnExitArgs;
#if defined (SYSLINK_USE_SYSMGR)
    ProcMgr_StopParams      stop_params;
#endif

    Osal_printf ("\nEntering RcmTestCleanup()\n");

    // send terminate message
    // allocate a remote command message
    rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
    rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
    if (rcmMsg == NULL) {
        Osal_printf ("RcmTestCleanup: Error allocating RCM message\n");
        goto exit;
    }

    // fill in the remote command message
    rcmMsg->fxnIdx = fxnExitIdx;
    fxnExitArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
    fxnExitArgs->a = 0xFFFF;

    // execute the remote command message
    Osal_printf ("RcmTestCleanup: calling RcmClient_execDpc \n");
    status = RcmClient_execDpc (rcmClientHandle, rcmMsg);
    if (status < 0) {
        Osal_printf ("RcmClientCleanup: RcmClient_execDpc error [0x%x]\n"
                            , status);
        goto exit;
    }

    // return message to the heap
    Osal_printf ("RcmTestCleanup: calling RcmClient_free \n");
    RcmClient_free (rcmClientHandle, rcmMsg);

    /* delete the rcm client */
    Osal_printf ("Delete RCM client instance \n");
    status = RcmClient_delete (&rcmClientHandle);
    if (status < 0)
        Osal_printf ("Error in RCM Client instance delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmClient_delete status: [0x%x]\n", status);

    /* rcm client module destroy*/
    Osal_printf ("Destroy RCM client module \n");
    status = RcmClient_destroy ();
    if (status < 0)
        Osal_printf ("Error in RCM Client module destroy [0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmClient_destroy status: [0x%x]\n", status);

    /* delete the rcm client */
    Osal_printf ("Delete RCM server instance \n");
    status = RcmServer_delete (&rcmServerHandle);
    if (status < 0)
        Osal_printf ("Error in RCM Server instance delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmServer_delete status: [0x%x]\n", status);

    /* rcm client module destroy*/
    Osal_printf ("estroy RCM server module \n");
    status = RcmServer_destroy ();
    if (status < 0)
        Osal_printf ("Error in RCM Server module destroy  [0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmServer_destroy status: [0x%x]\n", status);

    status = GatePeterson_close (&gateHandle_app_heap);
    if (status < 0)
        Osal_printf ("Error in GatePeterson_close [0x%x]\n", status);
    else
        Osal_printf ("GatePeterson_close status: [0x%x]\n", status);

    status = HeapBuf_close (&heapHandle_app_heap);
    if (status < 0)
        Osal_printf ("Error in HeapBuf_close [0x%x]\n", status);
    else
        Osal_printf ("HeapBuf_close status: [0x%x]\n", status);

    SharedRegion_remove (2);

    /* Finalize modules */
#if defined (SYSLINK_USE_SYSMGR)
    SharedRegion_remove (0);
    SharedRegion_remove (1);

    stop_params.proc_id = remoteId;
    ProcMgr_stop(procMgrHandle, &stop_params);

    status = ProcMgr_close (&procMgrHandle);
    if (status < 0)
        Osal_printf ("Error in ProcMgr_close [0x%x]\n", status);
    else
        Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

    SysMgr_destroy ();
 #else /* if defined (SYSLINK_USE_SYSMGR) */

    status = MessageQTransportShm_delete (&transportShmHandle);
    if (status < 0)
        Osal_printf ("Error in MessageQTransportShm_delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("MessageQTransportShm_delete status: [0x%x]\n"
                            , status);

    status = NameServerRemoteNotify_delete (&nsrnHandle);
    if (status < 0)
        Osal_printf ("Error in NameServerRemoteNotify_delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("NameServerRemoteNotify_delete status: [0x%x]\n"
                            , status);

    if (testCase == 1)
        status = MessageQ_unregisterHeap (HEAPID_SYSM3);
    else if (testCase == 2)
        status = MessageQ_unregisterHeap (HEAPID_APPM3);
    if (status < 0)
        Osal_printf ("Error in MessageQ_unregisterHeap [0x%x]\n"
                            , status);
    else
        Osal_printf ("MessageQ_unregisterHeap status: [0x%x]\n"
                            , status);

    status = HeapBuf_delete (&heapHandle);
    if (status < 0)
        Osal_printf ("Error in HeapBuf_delete [0x%x]\n", status);
    else
        Osal_printf ("HeapBuf_delete status: [0x%x]\n", status);

    status = NotifyDriverShm_delete (&notifyDrvHandle);
    if (status < 0)
        Osal_printf ("Error in NotifyDriverShm_delete [0x%x]\n", status);
    else
        Osal_printf ("NotifyDriverShm_delete status: [0x%x]\n", status);

    SharedRegion_remove (0);
    SharedRegion_remove (1);

    status = GatePeterson_close (&gateHandle_client);
    if (status < 0)
        Osal_printf ("Error in GatePeterson_close [0x%x]\n", status);
    else
        Osal_printf ("GatePeterson_close status: [0x%x]\n", status);

    status = GatePeterson_delete (&gateHandle_server);
    if (status < 0)
        Osal_printf ("Error in GatePeterson_delete [0x%x]\n", status);
    else
        Osal_printf ("GatePeterson_delete status: [0x%x]\n", status);

    status = ProcMgr_close (&procMgrHandle);
    if (status < 0)
        Osal_printf ("Error in ProcMgr_close [0x%x]\n", status);
    else
        Osal_printf ("ProcMgr_close status: [0x%x]\n", status);


    status = MessageQTransportShm_destroy ();
    if (status < 0)
        Osal_printf ("Error in MessageQTransportShm_destroy [0x%x]\n"
                            , status);
    else
        Osal_printf ("MessageQTransportShm_destroy status: [0x%x]\n"
                            , status);

    status = MessageQ_destroy ();
    if (status < 0)
        Osal_printf ("Error in MessageQ_destroy [0x%x]\n", status);
    else
        Osal_printf ("MessageQ_destroy status: [0x%x]\n", status);

    status = NotifyDriverShm_destroy ();
    if (status < 0)
        Osal_printf ("Error in NotifyDriverShm_destroy [0x%x]\n", status);
    else
        Osal_printf ("NotifyDriverShm_destroy status: [0x%x]\n", status);

    status = Notify_destroy ();
    if (status < 0)
        Osal_printf ("Error in Notify_destroy [0x%x]\n", status);
    else
        Osal_printf ("Notify_destroy status: [0x%x]\n", status);

    status = HeapBuf_destroy ();
    if (status < 0)
        Osal_printf ("Error in HeapBuf_destroy [0x%x]\n", status);
    else
        Osal_printf ("HeapBuf_destroy status: [0x%x]\n", status);

    status = ListMPSharedMemory_destroy ();
    if (status < 0)
        Osal_printf ("Error in ListMPSharedMemory_destroy [0x%x]\n"
                            , status);
    else
        Osal_printf ("ListMPSharedMemory_destroy status: [0x%x]\n"
                            , status);

    status = GatePeterson_destroy ();
    if (status < 0)
        Osal_printf ("Error in GatePeterson_destroy [0x%x]\n", status);
    else
        Osal_printf ("GatePeterson_destroy status: [0x%x]\n", status);

    status = SharedRegion_destroy ();
    if (status < 0)
        Osal_printf ("Error in SharedRegion_destroy [0x%x]\n", status);
    else
        Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);

    status = NameServerRemoteNotify_destroy ();
    if (status < 0)
        Osal_printf ("Error in NameServerRemoteNotify_destroy [0x%x]\n"
                            , status);
    else
        Osal_printf ("NameServerRemoteNotify_destroy status: [0x%x]\n"
                            , status);

    status = NameServer_destroy ();
    if (status < 0)
        Osal_printf ("Error in NameServer_destroy [0x%x]\n", status);
    else
        Osal_printf ("NameServer_destroy status: [0x%x]\n", status);

    UsrUtilsDrv_destroy ();

    status = MultiProc_destroy ();
    if (status < 0)
        Osal_printf ("Error in Multiproc_destroy [0x%x]\n", status);
    else
        Osal_printf ("Multiproc_destroy status: [0x%x]\n", status);
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

exit:
    Osal_printf ("Leaving RcmTestCleanup()\n");
    return status;
}

/*
 *  ======== RunServerTestThread ========
 *     RCM server test thread function
*/
Void StartRcmTestThreads (Int testCase)
{
    sem_init (&serverThreadWait, 0, 0);
    sem_init (&clientThreadWait, 0, 0);

    /* create the server thread */
    Osal_printf ("RunServerTestThread: Create server thread.\n");
    pthread_create (&thread_server, NULL, (Void *)&RcmServerThreadFxn,
                    (Void *) testCase);

    /* create the server thread */
    Osal_printf ("RunServerTestThread: Create client thread.\n");
    pthread_create (&thread_client, NULL, (Void *)&RcmClientThreadFxn,
                    (Void *) testCase);

    return;
}

/*
 *  ======== RunTest ========
 */
Int RunTest (Int testCase)
{
    Int status = 0;

    Osal_printf ("Testing RCM server on MPU\n");

    status = ipc_setup(testCase);

    StartRcmTestThreads (testCase);

    /* wait until signaled to delete the rcm server */
    Osal_printf ("RunTest: Wait for server thread completion.\n");
    sem_wait (&serverThreadWait);

    Osal_printf ("RunTest: Wait for client thread completion.\n");
    sem_wait (&clientThreadWait);

    pthread_join(thread_server, NULL);
    pthread_join(thread_client, NULL);

    status = RcmTestCleanup (testCase);
    if (status < 0)
        Osal_printf ("Error in RcmTestCleanup \n");

    Osal_printf ("RunTest: Leaving RunTest()\n");
    return status;
}

/*
 *  ======== main ========
 */
Int main (Int argc, Char * argv [])
{
    Int status = 0;
    Int testNo;

    Osal_printf("\nmain: == RCM Client and Server Sample ==\n");

    if (argc < 2)
    {
        Osal_printf("Usage: ./rcm_multitest.out <Test#>\n:\n");
        Osal_printf("\t./rcm_multitest.out 1 : MPU <--> SysM3 tesing\n");
        Osal_printf("\t./rcm_muiltitest.out 2 : MPU <--> AppM3 testing\n");
        goto exit;
    }

    testNo = atoi (argv[1]);

    /* Run RCM client and server test */
    Osal_printf ("main: RCM client and server test invoked\n");
    status = RunTest (testNo);
    if (status < 0)
        Osal_printf ("main: Error in RCM Client Server test \n");

exit:
    Osal_printf ("\n== Sample End ==\n");
    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
