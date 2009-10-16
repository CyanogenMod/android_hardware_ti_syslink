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
 *  @file   Server.c
 *
 *  @brief  RCM Server Sample application for RCM module between MPU & Ducati
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
#include <RcmServer.h>

/* Sample headers */
#include <Server.h>
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
#define BASE_DSP2ARM_INTID      26

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          send events.
 */
#define BASE_ARM2DSP_INTID      55

typedef struct {
    Int a;
} RCM_Remote_FxnDoubleArgs;

RcmServer_Handle                rcmServerHandle;
NotifyDriverShm_Handle          notifyDrvHandle_server;
GatePeterson_Handle             gateHandle_server;
NameServerRemoteNotify_Handle   nsrnHandle_server;
MessageQTransportShm_Handle     transportShmHandle_server;
ProcMgr_Handle                  procMgrHandle_server;
pthread_t                       thread; /* server thread object */
sem_t                           serverThreadSync;
sem_t                           mainThreadWait;
Int                             status;
Int                             testCase = 0;
UInt16                          remoteId_server;


/*
 *  ======== RCM_Remote_fxnDouble ========
 */
Int32 fxnDouble (UInt32 dataSize, UInt32 *data)
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
Int32 fxnExit (UInt32 dataSize, UInt32 *data)
{
    Osal_printf ("Executing Remote Function RCM_Remote_fxnExit \n");
    sem_post (&serverThreadSync);
    return status;
}

/*
 *  ======== RcmServerThreadFxn ========
 *     RCM server test thread function
*/
Void RcmServerThreadFxn (Void *arg)
{
    RcmServer_Config                cfgParams;
    RcmServer_Params                rcmServer_Params;
    Char *                          rcmServerName = RCMSERVER_NAME;
    UInt                            fxnIdx;
    Char *                          procName;
    UInt16                          procId;
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
    UInt32                          curShAddr;
    UInt32                          nsrnEventNo;
    UInt32                          mqtEventNo;
#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
    UInt32                          entry_point = 0;
    ProcMgr_StartParams             start_params;
#endif
#if defined (SYSLINK_USE_LOADER)
    Char *                          sysm3_image_name;
    Char *                          appm3_image_name;
    Char                            uProcId;
    UInt32                          fileId;
#endif
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config                   config;
#else
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
    MessageQTransportShm_Params     msgqTransportParams;
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    testCase = (Int) arg;

    switch(testCase) {
    case 1:
        Osal_printf ("RCM test with RCM client on Sys M3\n\n");
        procName = SYSM3_PROC_NAME;
        nsrnEventNo = NSRN_NOTIFY_EVENTNO;
        mqtEventNo = TRANSPORT_NOTIFY_EVENTNO;
        break;
    case 2:
        Osal_printf ("RCM test with RCM client on App M3\n\n");
        procName = APPM3_PROC_NAME;
        nsrnEventNo = NSRN_NOTIFY_EVENTNO1;
        mqtEventNo = TRANSPORT_NOTIFY_EVENTNO1;
        break;
    default:
        Osal_printf ("Please pass valid arg (1-Sys M3, 2-App M3) \n\n");
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
    status = MultiProc_setup (&multiProcConfig);
    if (status < 0) {
        Osal_printf ("Error in MultiProc_setup [0x%x]\n", status);
        goto exit;
    }
#endif

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
        Osal_printf ("Error in NameServer_setup [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("NameServer_setup Status [0x%x]\n", status);

    NameServerRemoteNotify_getConfig (&nsrConfig);
    status = NameServerRemoteNotify_setup (&nsrConfig);
    if (status < 0) {
        Osal_printf ("Error in NameServerRemoteNotify_setup [0x%x]\n",
                        status);
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
    status = SharedRegion_setup (&sharedRegConfig);
    if (status < 0) {
        Osal_printf ("Error in SharedRegion_setup. Status [0x%x]\n",
                     status);
        goto exit;
    }
    Osal_printf ("SharedRegion_setup Status [0x%x]\n", status);

    /* ListMPSharedMemory module setup */
    ListMPSharedMemory_getConfig (&listMPSharedConfig);
    status = ListMPSharedMemory_setup (&listMPSharedConfig);
    if (status < 0) {
        Osal_printf ("Error in ListMPSharedMemory_setup."
                       " Status [0x%x]\n", status);
        goto exit;
        }
    Osal_printf ("ListMPSharedMemory_setup Status [0x%x]\n", status);

    /* HeapBuf module setup */
    HeapBuf_getConfig (&heapbufConfig);
    status = HeapBuf_setup (&heapbufConfig);
    if (status < 0) {
    Osal_printf ("Error in HeapBuf_setup. Status [0x%x]\n",
         status);
         goto exit;
    }
    Osal_printf ("HeapBuf_setup Status [0x%x]\n", status);

    /* GatePeterson module setup */
    GatePeterson_getConfig (&gpConfig);
    status = GatePeterson_setup (&gpConfig);
    if (status < 0) {
        Osal_printf ("Error in GatePeterson_setup. Status [0x%x]\n",
                        status);
        goto exit;
    }
    Osal_printf ("GatePeterson_setup Status [0x%x]\n", status);

    /* Setup Notify module and NotifyDriverShm module */
    Notify_getConfig (&notifyConfig);
    notifyConfig.maxDrivers = 2;
    status = Notify_setup (&notifyConfig);
    if (status < 0) {
        Osal_printf ("Error in Notify_setup [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("Notify_setup Status [0x%x]\n", status);

    NotifyDriverShm_getConfig (&notifyDrvShmConfig);
    status = NotifyDriverShm_setup (&notifyDrvShmConfig);
    if (status < 0) {
        Osal_printf ("Error in NotifyDriverShm_setup [0x%x]\n",
                       status);
        goto exit;
    }
    Osal_printf ("NotifyDriverShm_setup Status [0x%x]\n", status);

    /* Setup MessageQ module and MessageQTransportShm module */
    MessageQ_getConfig (&messageqConfig);
    status = MessageQ_setup (&messageqConfig);
    if (status < 0) {
        Osal_printf ("Error in MessageQ_setup [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("MessageQ_setup Status [0x%x]\n", status);

    MessageQTransportShm_getConfig (&msgqTransportConfig);
    status = MessageQTransportShm_setup (&msgqTransportConfig);
    if (status < 0) {
        Osal_printf ("Error in MessageQTransportShm_setup [0x%x]\n",
                        status);
        goto exit;
    }
    Osal_printf ("MessageQTransportShm_setup Status [0x%x]\n",
                      status);

#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    /* Open a handle to the ProcMgr instance. */
    procId = MultiProc_getId (SYSM3_PROC_NAME);
    remoteId_server = MultiProc_getId (procName);
    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle_server,
                           procId);
        if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_open Status [0x%x]\n", status);

    /* Get the address of the shared region in kernel space. */
    status = ProcMgr_translateAddr (procMgrHandle_server,
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
    status = ProcMgr_translateAddr (procMgrHandle_server,
                                    (Ptr) &shAddrBase1,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM1,
                                    ProcMgr_AddrType_SlaveVirt);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("Virt address of shared address base #2:"
                            " [0x%x]\n", shAddrBase1);

        curShAddr = shAddrBase;
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (0,
                                   (Ptr) curShAddr,
                                   SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
            goto exit;
        }
    Osal_printf ("SharedRegion_add [0x%x]\n", status);

    /* Add the region to SharedRegion module. */
    status = SharedRegion_add (1,
                               (Ptr) shAddrBase1,
                               SHAREDMEMSIZE1);
    if (status < 0) {
        Osal_printf ("Error in SharedRegion_add1 [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("SharedRegion_add1 [0x%x]\n", status);

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
    notifyDrvHandle_server = NotifyDriverShm_create (
                                             "NOTIFYDRIVER_DUCATI",
                                             &notifyShmParams);
    Osal_printf ("NotifyDriverShm_create Handle: [0x%x]\n",
                 notifyDrvHandle_server);
    if (notifyDrvHandle_server == NULL) {
        Osal_printf ("Error in NotifyDriverShm_create\n");
        goto exit;
    }
#endif

#if defined(SYSLINK_USE_LOADER)
    if (testCase == 1)
        sysm3_image_name = "./RCMClient_MPUSYS_Test_Core0.xem3";
    else if (testCase == 2)
        sysm3_image_name = "./Notify_MPUSYS_reroute_Test_Core0.xem3";
    uProcId = PROC_SYSM3;
    Osal_printf ("loading the image %s\n", sysm3_image_name);
    Osal_printf ("uProcId = %d\n", uProcId);

    status = ProcMgr_load (procMgrHandle_server, sysm3_image_name, 2,
                            &sysm3_image_name, &entry_point, &fileId, uProcId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_load SysM3 image [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_load SysM3 image Status [0x%x]\n", status);
#endif
#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
    start_params.proc_id = PROC_SYSM3;
    Osal_printf("start_params.proc_id = %d\n", start_params.proc_id);

    status = ProcMgr_start (procMgrHandle_server, entry_point, &start_params);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_start SysM3 [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("ProcMgr_start Status [0x%x]\n", status);
#endif

    if(remoteId_server == PROC_APPM3) {
#if defined(SYSLINK_USE_LOADER)
        appm3_image_name = "./RCMClient_MPUAPP_Test_Core1.xem3";
        uProcId = remoteId_server;
        Osal_printf ("APPM3 Load: loading the APPM3 image %s\n",
                    appm3_image_name);
        Osal_printf ("APPM3 Load: uProcId = %d\n", uProcId);
        status = ProcMgr_load (procMgrHandle_server, appm3_image_name, 2,
                              &appm3_image_name, &entry_point, &fileId,
                              uProcId);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_load AppM3 image [0x%x]\n", status);
            goto exit;
        }
        Osal_printf ("ProcMgr_load AppM3 image Status [0x%x]\n", status);
#endif
#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
        start_params.proc_id = PROC_APPM3;
        Osal_printf("APPM3 Load: start_params.proc_id = %d\n",
                    start_params.proc_id);

        status = ProcMgr_start (procMgrHandle_server, entry_point,
                                &start_params);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_start AppM3 [0x%x]\n", status);
            goto exit;
        }
        Osal_printf ("ProcMgr_start Status [0x%x]\n", status);
#endif
    }

#if !defined(SYSLINK_USE_SYSMGR)
    GatePeterson_Params_init (gateHandle_server, &gateParams);
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
    gateParams.sharedAddr     = (Ptr)(curShAddr);

    Osal_printf ("Memory required for GatePeterson instance [0x%x]"
                 " bytes \n",
                 gateParams.sharedAddrSize);

    gateHandle_server = GatePeterson_create (&gateParams);
    if (gateHandle_server == NULL) {
        Osal_printf ("Error in GatePeterson_create [0x%x]\n", status);
        goto exit;
    }
    Osal_printf ("GatePeterson_create Status [0x%x]\n", status);

    /* Increment the offset for the next allocation */
    curShAddr = curShAddr + HEAPBUFMEMSIZE + GATEPETERSONMEMSIZE;

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
    nsrParams.notifyDriver  = notifyDrvHandle_server;
    nsrParams.notifyEventNo = nsrnEventNo;
    nsrParams.sharedAddr    = (Ptr)curShAddr;
    nsrParams.gate          = (Ptr)gateHandle_server;
    nsrParams.sharedAddrSize  = NSRN_MEMSIZE;
    nsrnHandle_server = NameServerRemoteNotify_create (remoteId_server, &nsrParams);
    if (nsrnHandle_server == NULL) {
        Osal_printf ("Error in NotifyDriverShm_create\n");
        goto exit;
    }
    Osal_printf ("NameServerRemoteNotify_create handle [0x%x]\n",
                         nsrnHandle_server);

    Osal_printf ("\nPlease break the platform and load the Ducati image now."
                   "Press any key to continue ...\n");
    getchar ();

    /* Increment the offset for the next allocation */
    curShAddr += NSRN_MEMSIZE;

    MessageQTransportShm_Params_init (NULL, &msgqTransportParams);

    msgqTransportParams.sharedAddr = (Ptr)curShAddr;
    msgqTransportParams.gate = (Gate_Handle) gateHandle_server;
    msgqTransportParams.notifyEventNo = mqtEventNo;
    msgqTransportParams.notifyDriver = notifyDrvHandle_server;
    msgqTransportParams.sharedAddrSize =
             MessageQTransportShm_sharedMemReq (&msgqTransportParams);

    transportShmHandle_server = MessageQTransportShm_create (remoteId_server,
                                                    &msgqTransportParams);
    if (transportShmHandle_server == NULL) {
        Osal_printf ("Error in MessageQTransportShm_create\n");
        goto exit;
    }
    Osal_printf ("MessageQTransportShm_create handle [0x%x]\n",
                     transportShmHandle_server);
#endif

    /* Get default config for rcm client module */
    Osal_printf ("Get default config for rcm server module.\n");
    status = RcmServer_getConfig (&cfgParams);
    if (status < 0) {
        Osal_printf ("Error in RCM Server module get config \n");
        goto exit;
    }
    Osal_printf ("RCM Client module get config passed \n");

    /* rcm client module setup*/
    Osal_printf ("RCM Server module setup.\n");
    status = RcmServer_setup (&cfgParams);
    if (status < 0) {
        Osal_printf ("Error in RCM Server module setup \n");
        goto exit;
    }
    Osal_printf ("RCM Server module setup passed \n");

    /* rcm client module params init*/
    Osal_printf ("rcm client module params init.\n");
    status = RcmServer_Params_init (NULL, &rcmServer_Params);
    if (status < 0) {
        Osal_printf ("Error in RCM Server instance params init \n");
        goto exit;
    }
    Osal_printf ("RCM Server instance params init passed \n");

    /* create the RcmServer instance */
    Osal_printf ("Creating RcmServer instance.\n");
    status = RcmServer_create (rcmServerName, &rcmServer_Params,
                                &rcmServerHandle);
    if (status < 0) {
        Osal_printf ("Error in RCM Server create.\n");
        goto exit;
    }
    Osal_printf ("RCM Server Create passed \n");

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

    sem_post (&mainThreadWait);

exit:
    Osal_printf ("Leaving RCM server test thread function \n");
    return;
}

/*
 *  ======== RcmServerCleanup ========
 */
Void RcmServerCleanup (Void)
{
#if defined (SYSLINK_USE_SYSMGR)
    ProcMgr_StopParams stop_params;
#endif


    /* delete the rcm client */
    Osal_printf ("Delete RCM server instance \n");
    status = RcmServer_delete (&rcmServerHandle);
    if (status < 0)
        Osal_printf ("Error in RCM Server instance delete[0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmServer_delete status: [0x%x]\n", status);

    /* rcm client module destroy*/
    Osal_printf ("Destroy RCM server module \n");
    status = RcmServer_destroy ();
    if (status < 0)
        Osal_printf ("Error in RCM Server module destroy [0x%x]\n"
                            , status);
    else
        Osal_printf ("RcmServer_destroy status: [0x%x]\n", status);

    /* Finalize modules */
#if defined (SYSLINK_USE_SYSMGR)
    SharedRegion_remove (0);
    SharedRegion_remove (1);

    stop_params.proc_id = remoteId_server;
    status = ProcMgr_stop(procMgrHandle_server, &stop_params);
    if (status < 0)
        Osal_printf("Error in ProcMgr_stop [0x%x]\n", status);
    else
        Osal_printf("ProcMgr_stop status: [0x%x]\n", status);

    if (testCase == 2) {
        stop_params.proc_id = PROC_SYSM3;
        status = ProcMgr_stop(procMgrHandle_server, &stop_params);
        if (status < 0)
            Osal_printf("Error in ProcMgr_stop [0x%x]\n", status);
        else
            Osal_printf("ProcMgr_stop status: [0x%x]\n", status);
    }

    status = ProcMgr_close (&procMgrHandle_server);
    if (status < 0)
        Osal_printf ("Error in ProcMgr_close [0x%x]\n", status);
    else
        Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

    status = SysMgr_destroy ();
    if (status < 0)
        Osal_printf("Error in SysMgr_destroy [0x%x]\n", status);
    else
        Osal_printf("SysMgr_destroy status: [0x%x]\n", status);

 #else /* if defined (SYSLINK_USE_SYSMGR) */
    status = MessageQTransportShm_delete (&transportShmHandle_server);
    if (status < 0)
        Osal_printf ("Error in MessageQTransportShm_delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("MessageQTransportShm_delete status: [0x%x]\n"
                            , status);

    status = NameServerRemoteNotify_delete (&nsrnHandle_server);
    if (status < 0)
        Osal_printf ("Error in NameServerRemoteNotify_delete [0x%x]\n"
                            , status);
    else
        Osal_printf ("NameServerRemoteNotify_delete status: [0x%x]\n"
                            , status);

    status = NotifyDriverShm_delete (&notifyDrvHandle_server);
    if (status < 0)
        Osal_printf ("Error in NotifyDriverShm_delete [0x%x]\n", status);
    else
        Osal_printf ("NotifyDriverShm_delete status: [0x%x]\n", status);

    SharedRegion_remove (0);
    SharedRegion_remove (1);

    status = GatePeterson_delete (&gateHandle_server);
    if (status < 0)
        Osal_printf ("Error in GatePeterson_delete [0x%x]\n", status);
    else
        Osal_printf ("GatePeterson_delete status: [0x%x]\n", status);

    status = ProcMgr_close (&procMgrHandle_server);
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

    status = ListMPSharedMemory_destroy();
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
}

/*
 *  ======== RunServerTestThread ========
 *     RCM server test thread function
*/
Void StartRcmServerTestThread (Int testCase)
{
    sem_init (&mainThreadWait, 0, 0);

    /* create the server thread */
    Osal_printf ("Create server thread.\n");
    pthread_create (&thread, NULL, (Void *)&RcmServerThreadFxn,
                    (Void *) testCase);

    return;
}

/*
 *  ======== MpuRcmServerTest ========
 */
Int MpuRcmServerTest (Int testCase)
{
    Osal_printf ("Testing RCM server on MPU\n");

    StartRcmServerTestThread (testCase);

    /* wait until signaled to delete the rcm server */
    Osal_printf ("Wait for server thread completion.\n");
    sem_wait (&mainThreadWait);

    pthread_join (thread, NULL);

    RcmServerCleanup ();

    Osal_printf ("Leaving MpuRcmServerTest()\n");
    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
