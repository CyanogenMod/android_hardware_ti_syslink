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
/*==============================================================================
 *  @file   MessageQApp.c
 *
 *  @brief  Sample application for MessageQ module between MPU & SysM3
 *
 *  ============================================================================
 */


/* Standard headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <OsalPrint.h>
#include <Memory.h>
#include <String.h>

/* Module level headers */
#if defined (SYSLINK_USE_SYSMGR)
#include <SysMgr.h>
#else /* if defined (SYSLINK_USE_SYSMGR) */
#include <UsrUtilsDrv.h>
#include <MultiProc.h>
#include <NameServer.h>
#include <SharedRegion.h>
#include <GatePeterson.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>
#include <Heap.h>
#include <HeapBuf.h>
#include <Notify.h>
#include <NotifyDriverShm.h>
#include <NameServerRemoteNotify.h>
#include <MessageQ.h>
#include <ProcMgr.h>
#endif /* if defined(SYSLINK_USE_SYSMGR) */

/* Application header */
#include "MessageQApp_config.h"


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
 *  @brief  Number of transfers to be tested.
 */
//#define  MESSAGEQAPP_NUM_TRANSFERS  1000
#define  MESSAGEQAPP_NUM_TRANSFERS  10


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
NameServerRemoteNotify_Handle  MessageQApp_nsrnHandle;
NotifyDriverShm_Handle         MessageQApp_notifyDrvHandle;
MessageQ_Handle                MessageQApp_messageQ;
GatePeterson_Handle            MessageQApp_gateHandle;
HeapBuf_Handle                 MessageQApp_heapHandle;
MessageQTransportShm_Handle    MessageQApp_transportShmHandle;
MessageQ_QueueId               MessageQApp_queueId;
UInt16                         MessageQApp_procId;
UInt32                         MessageQApp_shAddrBase;
UInt32                         MessageQApp_curShAddr;
ProcMgr_Handle                 MessageQApp_procMgrHandle;

/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to execute the startup for MessageQApp sample application
 *
 *  @sa
 */
Int
MessageQApp_startup (UInt32 sharedAddr)
{
    Int32                          status  = 0;
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config                  config;
#else
    Notify_Config                  notifyConfig;
    NotifyDriverShm_Config         notifyDrvShmConfig;
    NameServerRemoteNotify_Config  nsrConfig;
    SharedRegion_Config            sharedRegConfig;
    ListMPSharedMemory_Config      listMPSharedConfig;
    GatePeterson_Config            gpConfig;
    HeapBuf_Config                 heapbufConfig;
    MultiProc_Config               multiProcConfig;
    NotifyDriverShm_Params         notifyShmParams;
    GatePeterson_Params            gateParams;
    NameServerRemoteNotify_Params  nsrParams;
    HeapBuf_Params                 heapbufParams;
    MessageQ_Config                messageqConfig;
    MessageQTransportShm_Config    msgqTransportConfig;
    MessageQTransportShm_Params    msgqTransportParams;
#endif /* if defined(SYSLINK_USE_SYSMGR) */
#if !defined (SYSLINK_USE_DAEMON)
#if defined(SYSLINK_USE_LOADER) || defined(SYSLINK_USE_SYSMGR)
    UInt32                         entry_point = 0;
    ProcMgr_StartParams            start_params;
#endif
#if defined(SYSLINK_USE_LOADER)
    Char *                         image_name;
    UInt32                         fileId;
#endif /* if defined(SYSLINK_USE_LOADER) */
#endif /* if !defined(SYSLINK_USE_DAEMON) */

    Osal_printf ("Entered MessageQApp_startup sharedAddr [0x%x]\n",
                 sharedAddr);

    MessageQApp_shAddrBase = sharedAddr;

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }
#else /* if defined(SYSLINK_USE_SYSMGR) */
    if (status >= 0) {
       UsrUtilsDrv_setup ();

        /* Get and set GPP MultiProc ID by name. */
        multiProcConfig.maxProcessors = 4;
        multiProcConfig.id = 0;
        String_cpy (multiProcConfig.nameList [0], "MPU");
        String_cpy (multiProcConfig.nameList [1], "Tesla");
        String_cpy (multiProcConfig.nameList [2], "SysM3");
        String_cpy (multiProcConfig.nameList [3], "AppM3");
        status = MultiProc_setup(&multiProcConfig);
        if (status < 0) {
            Osal_printf ("Error in MultiProc_setup [0x%x]\n", status);
        }

        /* NameServer and NameServerRemoteNotify module setup */
        status = NameServer_setup ();
        if (status < 0) {
            Osal_printf ("Error in NameServer_setup [0x%x]\n", status);
        }
        else {
            Osal_printf ("NameServer_setup Status [0x%x]\n", status);
            NameServerRemoteNotify_getConfig (&nsrConfig);
            status = NameServerRemoteNotify_setup (&nsrConfig);
            if (status < 0) {
                Osal_printf ("Error in NameServerRemoteNotify_setup [0x%x]\n",
                             status);
            }
            else {
                Osal_printf ("NameServerRemoteNotify_setup Status [0x%x]\n",
                             status);
            }
        }
    }

    if (status >= 0) {
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
            Osal_printf ("Error in SharedRegion_setup. Status [0x%x]\n",
                         status);
        }
        else {
            Osal_printf ("SharedRegion_setup Status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* ListMPSharedMemory module setup */
        ListMPSharedMemory_getConfig (&listMPSharedConfig);
        status = ListMPSharedMemory_setup (&listMPSharedConfig);
        if (status < 0) {
            Osal_printf ("Error in ListMPSharedMemory_setup."
                         " Status [0x%x]\n",
                         status);
        }
        else  {
            Osal_printf ("ListMPSharedMemory_setup Status [0x%x]\n",
                         status);

            /* HeapBuf module setup */
            HeapBuf_getConfig (&heapbufConfig);
            status = HeapBuf_setup (&heapbufConfig);
            if (status < 0) {
                Osal_printf ("Error in HeapBuf_setup. Status [0x%x]\n",
                status);
            }
            else {
                Osal_printf ("HeapBuf_setup Status [0x%x]\n", status);
            }
        }
    }

    if (status >= 0) {
        /* GatePeterson module setup */
        GatePeterson_getConfig (&gpConfig);
        status = GatePeterson_setup (&gpConfig);
        if (status < 0) {
            Osal_printf ("Error in GatePeterson_setup. Status [0x%x]\n",
                         status);
        }
        else {
            Osal_printf ("GatePeterson_setup Status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* Setup Notify module and NotifyDriverShm module */
        Notify_getConfig (&notifyConfig);
        notifyConfig.maxDrivers = 2;
        status = Notify_setup (&notifyConfig);
        if (status < 0) {
            Osal_printf ("Error in Notify_setup [0x%x]\n", status);
        }
        else {
            Osal_printf ("Notify_setup Status [0x%x]\n", status);
            NotifyDriverShm_getConfig (&notifyDrvShmConfig);
            status = NotifyDriverShm_setup (&notifyDrvShmConfig);
            if (status < 0) {
                Osal_printf ("Error in NotifyDriverShm_setup [0x%x]\n",
                             status);
            }
            else {
                Osal_printf ("NotifyDriverShm_setup Status [0x%x]\n", status);
            }
        }
    }

    if (status >= 0) {
        /* Setup MessageQ module and MessageQTransportShm module */
        MessageQ_getConfig (&messageqConfig);
        status = MessageQ_setup (&messageqConfig);
        if (status < 0) {
            Osal_printf ("Error in MessageQ_setup [0x%x]\n", status);
        }
        else {
            Osal_printf ("MessageQ_setup Status [0x%x]\n", status);
            MessageQTransportShm_getConfig (&msgqTransportConfig);
            status = MessageQTransportShm_setup (&msgqTransportConfig);
            if (status < 0) {
                Osal_printf ("Error in MessageQTransportShm_setup [0x%x]\n",
                             status);
            }
            else {
                Osal_printf ("MessageQTransportShm_setup Status [0x%x]\n",
                             status);
            }
        }
    }
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    MessageQApp_procId = MultiProc_getId ("SysM3");
    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&MessageQApp_procMgrHandle,
                           MessageQApp_procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        Osal_printf ("ProcMgr_open Status [0x%x]\n", status);
        /* Get the address of the shared region in kernel space. */
        status = ProcMgr_translateAddr (MessageQApp_procMgrHandle,
                                        (Ptr) &MessageQApp_shAddrBase,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);

        if (status < 0) {
            Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n", status);
        }
        else {
            Osal_printf ("Virt address of shared address base:"
                         " [0x%x]\n", MessageQApp_shAddrBase);
        }
    }

    if (status >= 0) {
        MessageQApp_curShAddr = MessageQApp_shAddrBase;
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (0,
                                   (Ptr) MessageQApp_shAddrBase,
                                   SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
        else {
            Osal_printf ("SharedRegion_add [0x%x]\n", status);
        }
    }

#if !defined(SYSLINK_USE_SYSMGR)
    if (status >= 0) {
        /* Create instance of NotifyDriverShm */
        NotifyDriverShm_Params_init (NULL, &notifyShmParams);
        // notifyShmParams.sharedAddr= (UInt32)MessageQApp_curShAddr;
        // notifyShmParams.sharedAddrSize = 0x4000;
        /* NotifyDriverShm_sharedMemReq (&notifyShmParams); */
        notifyShmParams.numEvents          = 32;
        notifyShmParams.numReservedEvents  = 0;
        notifyShmParams.sendEventPollCount = (UInt32) -1;
        notifyShmParams.recvIntId          = BASE_DSP2ARM_INTID;
        notifyShmParams.sendIntId          = BASE_ARM2DSP_INTID;
        notifyShmParams.remoteProcId       = MessageQApp_procId;

        /* Increment the offset for the next allocation */
        MessageQApp_curShAddr += NOTIFYMEMSIZE;

        /* Create instance of NotifyDriverShm */
        MessageQApp_notifyDrvHandle = NotifyDriverShm_create (
                                                     "NOTIFYDRIVER_DUCATI",
                                                     &notifyShmParams);
        Osal_printf ("NotifyDriverShm_create Handle: [0x%x]\n",
                     MessageQApp_notifyDrvHandle);

        if (MessageQApp_notifyDrvHandle == NULL) {
            Osal_printf ("Error in NotifyDriverShm_create\n");
        }
    }
#endif

#if defined(SYSLINK_USE_SYSMGR)
#if !defined (SYSLINK_USE_DAEMON)
    start_params.proc_id = MessageQApp_procId;
#ifdef SYSLINK_USE_LOADER
    image_name = "./MessageQ_MPUSYS_Test_Core0.xem3";
    Osal_printf ("Loading image (%s) onto Ducati with ProcId %d\n", image_name,
                start_params.proc_id);
    status = ProcMgr_load (MessageQApp_procMgrHandle, image_name, 2,
                            (String *)image_name, &entry_point, &fileId,
                            start_params.proc_id);
    Osal_printf ("ProcMgr_load SysM3 Status [0x%x]\n", status);
#endif /* SYSLINK_USE_LOADER */
    status = ProcMgr_start (MessageQApp_procMgrHandle, entry_point,
                            &start_params);
    Osal_printf ("ProcMgr_start SysM3 Status [0x%x]\n", status);
#endif /* !SYSLINK_USE_DAEMON */
#else /* Non SysMgr version */
#ifdef SYSLINK_USE_LOADER
#if !defined (SYSLINK_USE_DAEMON)
    start_params.proc_id = MessageQApp_procId;
    image_name = "./MessageQ_MPUSYS_Test_Core0.xem3";
    Osal_printf ("Loading image (%s) onto Ducati with ProcId %d\n", image_name,
                start_params.proc_id);
    status = ProcMgr_load (MessageQApp_procMgrHandle, image_name, 2,
                            (String *)image_name, &entry_point, &fileId,
                            start_params.proc_id);
    Osal_printf ("ProcMgr_load SysM3 Status [0x%x]\n", status);
    status = ProcMgr_start (MessageQApp_procMgrHandle, entry_point,
                            &start_params);
    Osal_printf ("ProcMgr_start SysM3 Status [0x%x]\n", status);
#endif /* !SYSLINK_USE_DAEMON */
#else
    Osal_printf ("Please break the platform and load the Ducati image now."
        "Press any key to continue ...\n");
    getchar ();
#endif /* SYSLINK_USE_LOADER */

    if (status >= 0) {
        Osal_printf ("Opening the Gate\n");

        GatePeterson_Params_init (MessageQApp_gateHandle, &gateParams);
        gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
        Osal_printf ("Memory required for GatePeterson instance [0x%x]"
                 " bytes \n",
                 gateParams.sharedAddrSize);

        do {
            gateParams.sharedAddr     = (Ptr)(MessageQApp_curShAddr);
            status = GatePeterson_open (&MessageQApp_gateHandle,
                                        &gateParams);
        }
        while ((status == GATEPETERSON_E_NOTFOUND) || 
                            (status == GATEPETERSON_E_VERSION));
        if (status < 0) {
            Osal_printf ("Error in GatePeterson_open [0x%x]\n", status);
        }
        else {
            Osal_printf ("GatePeterson_open Status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* Increment the offset for the next allocation */
        MessageQApp_curShAddr += GATEPETERSONMEMSIZE;
        Osal_printf ("Create the Heap: [0x%x]\n", MessageQApp_curShAddr);

        /* Create the heap. */
        HeapBuf_Params_init(NULL, &heapbufParams);
        heapbufParams.name           = HEAPNAME;
        heapbufParams.sharedAddr     = (Ptr)(MessageQApp_curShAddr);
        heapbufParams.align          = 128;
        heapbufParams.numBlocks      = 4;
        heapbufParams.blockSize      = MSGSIZE;
        heapbufParams.gate           = (Gate_Handle) MessageQApp_gateHandle;
        heapbufParams.sharedAddrSize = HeapBuf_sharedMemReq (&heapbufParams,
                                                &heapbufParams.sharedBufSize);
        heapbufParams.sharedBuf      = (Ptr) (   MessageQApp_curShAddr
                                           +  heapbufParams.sharedAddrSize);
        Osal_printf ("Before HeapBuf_Create: [0x%x]\n", MessageQApp_curShAddr);
        MessageQApp_heapHandle = HeapBuf_create (&heapbufParams);

        if(MessageQApp_heapHandle != NULL) {
            /* Register this heap with MessageQ */
            MessageQ_registerHeap (MessageQApp_heapHandle, HEAPID);
        } else {
            Osal_printf ("HeapBuf_create failed.\n");
            status = -1;
        }
    }

    if (status >= 0) {
        /* Increment the offset for the next allocation */
        MessageQApp_curShAddr += HEAPBUFMEMSIZE;
        /*
         *  Create the NameServerRemote implementation that is used to
         *  communicate with the remote processor. It uses some shared
         *  memory and the Notify module.
         *
         *  Note that this implementation uses Notify to communicate, so
         *  interrupts need to be enabled. On BIOS, that does not occur
         *  until after main returns.
         */
        NameServerRemoteNotify_Params_init(NULL, &nsrParams);
        nsrParams.notifyDriver  = MessageQApp_notifyDrvHandle;
        nsrParams.notifyEventNo = NSRN_NOTIFY_EVENTNO;
        nsrParams.sharedAddr    = (Ptr)MessageQApp_curShAddr;
        nsrParams.gate          = (Ptr)MessageQApp_gateHandle;
        nsrParams.sharedAddrSize  = NSRN_MEMSIZE;
        MessageQApp_nsrnHandle =
                     NameServerRemoteNotify_create(MessageQApp_procId,
                                                   &nsrParams);
        if (MessageQApp_nsrnHandle == NULL) {
            Osal_printf ("Error in NotifyDriverShm_create\n");
        }
        else {
            Osal_printf ("NameServerRemoteNotify_create handle [0x%x]\n",
                         MessageQApp_nsrnHandle);
        }
    }

    if (status >= 0) {
        /* Increment the offset for the next allocation */
        MessageQApp_curShAddr += NSRN_MEMSIZE;

        MessageQTransportShm_Params_init (NULL, &msgqTransportParams);

        msgqTransportParams.sharedAddr = (Ptr)MessageQApp_curShAddr;
        msgqTransportParams.gate = (Gate_Handle) MessageQApp_gateHandle;
        msgqTransportParams.notifyEventNo = TRANSPORT_NOTIFY_EVENTNO;
        msgqTransportParams.notifyDriver = MessageQApp_notifyDrvHandle;
        msgqTransportParams.sharedAddrSize =
                 MessageQTransportShm_sharedMemReq (&msgqTransportParams);

        MessageQApp_transportShmHandle =
                       MessageQTransportShm_create (MessageQApp_procId,
                                                    &msgqTransportParams);
        if (MessageQApp_transportShmHandle == NULL) {
            Osal_printf ("Error in MessageQTransportShm_create\n");
        }
        else {
            Osal_printf ("MessageQTransportShm_create handle [0x%x]\n",
                         MessageQApp_transportShmHandle);
        }
    }
#endif

    Osal_printf ("Leaving MessageQApp_startup\n");

    return (status);
}


/*!
 *  @brief  Function to execute the MessageQApp sample application
 *
 *  @sa
 */
Int
MessageQApp_execute (Void)
{
    Int32                    status = 0;
    MessageQ_Msg             msg    = NULL;
    MessageQ_Params          msgParams;
    UInt16                   i;

    Osal_printf ("Entered MessageQApp_execute\n");

    /* Create the Message Queue. */
    MessageQ_Params_init (NULL, &msgParams);
    MessageQApp_messageQ = MessageQ_create (ARM_MESSAGEQNAME, &msgParams);
    if (MessageQApp_messageQ == NULL) {
        Osal_printf ("Error in MessageQ_create\n");
    }
    else {
        Osal_printf ("MessageQ_create handle [0x%x]\n",
                     MessageQApp_messageQ);
    }

    if (status >=0) {
        do {
            status = MessageQ_open (DUCATI_CORE0_MESSAGEQNAME,
                                    &MessageQApp_queueId);
        } while (status == MESSAGEQ_E_NOTFOUND);
        if (status < 0) {
            Osal_printf ("Error in MessageQ_open [0x%x]\n", status);
        }
        else {
            Osal_printf ("MessageQ_open Status [0x%x]\n", status);
            Osal_printf ("MessageQApp_queueId  [0x%x]\n", MessageQApp_queueId);
        }
    }

    if (status > 0) {
        Osal_printf ("\nExchanging messages with remote processor\n");
        for (i = 0 ; i < MESSAGEQAPP_NUM_TRANSFERS ; i++) {
            /* Allocate message. */
            msg = MessageQ_alloc (HEAPID, MSGSIZE);
            if (msg == NULL) {
                Osal_printf ("Error in MessageQ_alloc\n");
                break;
            }
            else {
                Osal_printf ("MessageQ_alloc msg [0x%x]\n", msg);
            }

            MessageQ_setMsgId (msg, (i % 16));

            /* Have the DSP reply to this message queue */
            MessageQ_setReplyQueue (MessageQApp_messageQ, msg);

            status = MessageQ_put (MessageQApp_queueId, msg);
            if (status < 0) {
                Osal_printf ("Error in MessageQ_put [0x%x]\n",
                             status);
                break;
            }
            else {
                Osal_printf ("MessageQ_put #%d Status [0x%x]\n", i, status);
            }

            status = MessageQ_get(MessageQApp_messageQ, &msg, MESSAGEQ_FOREVER);
            if (status < 0) {
                Osal_printf ("Error in MessageQ_get\n");
                break;
            }
            else {
                /* Validate the returned message. */
                if (msg != NULL) {
                    if (MessageQ_getMsgId (msg) != ((i % 16) + 1) ) {
                        Osal_printf ("Data integrity failure!\n"
                                     "    Expected %d\n"
                                     "    Received %d\n",
                                     ((i % 16) + 1),
                                     MessageQ_getMsgId (msg));
                        break;
                    }
                }

                status = MessageQ_free (msg);
                Osal_printf ("MessageQ_free status [0x%x]\n", status);
            }

            if ((i % 2) == 0) {
                Osal_printf ("Exchanged %d messages with remote processor\n",
                             i);
            }
        }
    }

    /* Keep the Ducati application running. */
#if !defined (SYSLINK_USE_DAEMON)
    /* Send die message */
    msg = MessageQ_alloc (HEAPID, MSGSIZE);
    if (msg == NULL) {
        Osal_printf ("MessageQ_alloc (die message) failed\n");
    }
    else {
        Osal_printf ("MessageQ_alloc (die message) msg = [0x%x]\n", msg);

        /* Send a die message */
        MessageQ_setMsgId(msg, DIEMESSAGE);

        /* Have the DSP reply to this message queue */
        MessageQ_setReplyQueue (MessageQApp_messageQ, msg);

        /* Send the message off */
        status = MessageQ_put (MessageQApp_queueId, msg);
        if (status < 0) {
            Osal_printf ("Error in MessageQ_put (die message) [0x%x]\n",
                         status);
        }
        else {
            Osal_printf ("MessageQ_put (die message) Status [0x%x]\n", status);
        }

        /* Wait for the final message. */
        status = MessageQ_get(MessageQApp_messageQ, &msg, MESSAGEQ_FOREVER);
        if (status < 0) {
            Osal_printf ("\nError in MessageQ_get (die message)!\n");
        }
        else {
            if (msg != NULL) {
                /* Validate the returned message. */
                if (MessageQ_getMsgId(msg) == DIEMESSAGE) {
                    Osal_printf("\nSuccessfully received die response from the "
                                "remote  processor\n");
                    Osal_printf("Sample application successfully completed!\n");
                }
                else {
                    Osal_printf("\nUnsuccessful run of the sample "
                                "application!\n");
                }
            }
            else {
                Osal_printf("\nUnsuccessful run of the sample application msg "
                          "is NULL!\n");
            }
        }
        MessageQ_free(msg);
    }
#else
    Osal_printf ("Sample application successfully completed!\n");
#endif /* !SYSLINK_USE_DAEMON */

    /* Clean-up */
    if (MessageQApp_messageQ != NULL) {
        status = MessageQ_delete (&MessageQApp_messageQ);
        if (status < 0) {
            Osal_printf ("Error in MessageQ_delete [0x%x]\n",
                         status);
        }
        else {
            Osal_printf ("MessageQ_delete Status [0x%x]\n", status);
        }
    }

    if (MessageQApp_messageQ != NULL) {
        MessageQ_close (&MessageQApp_queueId);
    }

    Osal_printf ("Leaving MessageQApp_execute\n");

    return (status);
}


/*!
 *  @brief  Function to execute the shutdown for MessageQApp sample application
 *
 *  @sa
 */
Int
MessageQApp_shutdown (Void)
{
    Int32               status = 0;
#if !defined (SYSLINK_USE_DAEMON)
#if defined (SYSLINK_USE_SYSMGR)
    ProcMgr_StopParams  stop_params;
#endif
#endif /* !SYSLINK_USE_DAEMON */

    Osal_printf ("Entered MessageQApp_shutdown()\n");

#if defined (SYSLINK_USE_SYSMGR)
    SharedRegion_remove (0);

#if !defined (SYSLINK_USE_DAEMON)
    stop_params.proc_id = MessageQApp_procId;
    status = ProcMgr_stop (MessageQApp_procMgrHandle, &stop_params);
    Osal_printf ("ProcMgr_stop status: [0x%x]\n", status);
#endif /* !SYSLINK_USE_DAEMON */

    status = ProcMgr_close (&MessageQApp_procMgrHandle);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

    status = SysMgr_destroy ();
    Osal_printf ("SysMgr_destroy status: [0x%x]\n", status);
#else /* if defined (SYSLINK_USE_SYSMGR) */
    /* Finalize modules */
    status = MessageQTransportShm_delete(&MessageQApp_transportShmHandle);
    Osal_printf ("MessageQTransportShm_delete status: [0x%x]\n", status);

    status = NameServerRemoteNotify_delete (&MessageQApp_nsrnHandle);
    Osal_printf ("NameServerRemoteNotify_delete status: [0x%x]\n", status);

    status = MessageQ_unregisterHeap (HEAPID);
    Osal_printf ("MessageQ_unregisterHeap status: [0x%x]\n", status);

    status = HeapBuf_delete (&MessageQApp_heapHandle);
    Osal_printf ("HeapBuf_delete status: [0x%x]\n", status);

    status = NotifyDriverShm_delete (&MessageQApp_notifyDrvHandle);
    Osal_printf ("NotifyDriverShm_delete status: [0x%x]\n", status);

    SharedRegion_remove (0);

    status = GatePeterson_close (&MessageQApp_gateHandle);
    Osal_printf ("GatePeterson_close status: [0x%x]\n", status);

    status = MessageQTransportShm_destroy ();
    Osal_printf ("MessageQTransportShm_destroy status: [0x%x]\n", status);

    status = MessageQ_destroy ();
    Osal_printf ("MessageQ_destroy status: [0x%x]\n", status);

    status = NotifyDriverShm_destroy ();
    Osal_printf ("NotifyDriverShm_destroy status: [0x%x]\n", status);

    status = Notify_destroy ();
    Osal_printf ("Notify_destroy status: [0x%x]\n", status);

    status = HeapBuf_destroy ();
    Osal_printf ("HeapBuf_destroy status: [0x%x]\n", status);

    status = ListMPSharedMemory_destroy();
    Osal_printf ("ListMPSharedMemory_destroy status: [0x%x]\n", status);

    status = GatePeterson_destroy ();
    Osal_printf ("GatePeterson_destroy status: [0x%x]\n", status);

    status = SharedRegion_destroy ();
    Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);

    status = NameServerRemoteNotify_destroy ();
    Osal_printf ("NameServerRemoteNotify_destroy status: [0x%x]\n", status);

    status = NameServer_destroy ();
    Osal_printf ("NameServer_destroy status: [0x%x]\n", status);

    status = MultiProc_destroy ();
    Osal_printf ("Multiproc_destroy status: [0x%x]\n", status);

    UsrUtilsDrv_destroy ();
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Leave MessageQApp_shutdown()\n");

    return (status);
}
