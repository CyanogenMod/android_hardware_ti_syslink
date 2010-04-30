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

/*  ----------------------------------- Linux headers                 */
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
/* Standard headers */
#include <Std.h>
#include <errbase.h>
#include <ipctypes.h>

/*  ----------------------------------- Notify headers              */
#include <NotifyDriverShm.h>
#include <notifyxfer_os.h>
#include <NotifyDriverDefs.h>
#include <Notify.h>
#include <Trace.h>
#include <stdlib.h>
#include <String.h>
#include <OsalPrint.h>

#include <Memory.h>

#if defined (SYSLINK_USE_SYSMGR)
#include <SysMgr.h>
#else /* if defined (SYSLINK_USE_SYSMGR) */
#include <omap4430proc.h>
#include <UsrUtilsDrv.h>
#include <ProcMgr.h>
#include <MultiProc.h>
#endif /* if defined (SYSLINK_USE_SYSMGR) */

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @macro  EVENT_STARTNO
 *
 *  @desc   Number after resevred events.
 *  ============================================================================
 */
#define EVENT_STARTNO           3

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          receive events.
 */
#define NOTIFYAPP_RECV_INT_ID   26

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          send events.
 */
#define NOTIFYAPP_SEND_INT_ID   55

#define NOTIFYAPP_NUMEVENTS     0

#define NOTIFYAPP_EVENT_NO      32

#define NOTIFYAPP_NUM_TRANSFERS 100

#define MAX_EVENTS              32

#define NUM_MEM_ENTRIES         9

#define RESET_VECTOR_ENTRY_ID   0

#define NOTIFY_SYSM3_IMAGE_PATH "./Transport_MPUSYS_Test_PM_Core0.xem3"
#define NOTIFY_APPM3_IMAGE_PATH "./Transport_MPUAPP_Test_PM_Core1.xem3"

/** ============================================================================
 *  @macro  numIterations
 *
 *  @desc   Number of ietrations.
 *  ============================================================================
 */
UInt16 numIterations = NOTIFYAPP_NUMEVENTS;

UInt16 NotifyApp_recvEventCount [MAX_EVENTS];

Processor_Id procId;

PVOID           event [MAX_EVENTS];
UInt16          eventNo;
ProcMgr_Handle  procHandle;
Handle          proc4430Handle;

Memory_MapInfo  traceinfo;

typedef struct ccbMessage {
    unsigned ccb_flag : 1; /* this message uses a ccb */
    unsigned ccb_num : 6; /* ccb_num zero means NO ccb */
    unsigned reply_flag : 1; /* this message is a reply or acknowledgement */
    unsigned msg_type : 4;
    unsigned msg_subtype : 4;
    unsigned parm : 16;
} ccbMessage;

typedef union messageslicer {
    ccbMessage fields;
    Uint32     whole;
} messageslicer;


typedef struct ccbBlock {
    unsigned ccb_num  : 5;
    unsigned msg_type : 5;
    unsigned sub_type : 4;
    unsigned rqst_cpu : 4; /* CPU owning this CCB */
    unsigned fill     : 14;
    unsigned proc_id;
    unsigned *sem_hnd;
    unsigned mem_hnd;
    unsigned body[4];
} ccbBlock;

typedef unsigned catType;

typedef struct sms {
    catType CAT;
    ccbBlock ccb[33];
} sms;

sms *CCBStruct;
messageslicer ccbPayload;

typedef struct Notify_Ping_SemObj_tag {
    sem_t  sem;
} Notify_Ping_SemObj;

struct ping_arg{
    UInt32 arg1;
    UInt32 arg2;
};

struct ping_arg evt_cbk_arg[MAX_EVENTS];

NotifyDriverShm_Handle notifyAppHandle = NULL;

/** ----------------------------------------------------------------------------
 *  @name   NotifyPing_Execute
 *
 *  @desc   Function to execute the NotifyPing sample application
 *
 *  @arg    None
 *
 *  @ret    None
 *
 *  @enter  None.
 *
 *  @leave  None.
 *
 *  @see    None.
 *  ----------------------------------------------------------------------------
 */
static Int SlpmTransport_Execute (Void);
Int SlpmTransport_shutdown (Void);

Int NotifyPing_CreateSem (OUT PVOID * semPtr)
{
    Int status = NOTIFY_SUCCESS;
    Notify_Ping_SemObj * semObj;
    Int                  osStatus;

    semObj = malloc (sizeof (Notify_Ping_SemObj));
    if (semObj != NULL) {
        osStatus = sem_init (&(semObj->sem), 0, 0);
        if (osStatus < 0) {
            status = DSP_EFAIL;
        }
        else {
            *semPtr = (PVOID) semObj;
        }
    }
    else {
        *semPtr = NULL;
        status = DSP_EFAIL;
    }

    return status;
}


Void NotifyPing_DestroySem (IN PVOID * semHandle)
{
    Int status                  = NOTIFY_SUCCESS;
    Notify_Ping_SemObj * semObj = (Notify_Ping_SemObj *)semHandle;
    Int                  osStatus;

    osStatus = sem_destroy (&(semObj->sem));
    if (osStatus) {
        status = DSP_EFAIL;
    }
    else {
       free (semObj);
    }
    Osal_printf ("Notify_DestroySem status[%x]\n",status);
}


Int NotifyPing_PostSem (IN PVOID semHandle)
{
    Int                     status = NOTIFY_SUCCESS;
    Notify_Ping_SemObj *    semObj = semHandle;
    Int                     osStatus;

    osStatus = sem_post (&(semObj->sem));
    if (osStatus < 0) {
        status = DSP_EFAIL;
    }

    return status;
}


Int NotifyPing_WaitSem (IN PVOID semHandle)
{
    Int                  status = NOTIFY_SUCCESS;
    Notify_Ping_SemObj * semObj = semHandle;
    Int                  osStatus;

    Osal_printf ("NotifyPing_WaitSem\n");
    osStatus = sem_wait (&(semObj->sem));
    if (osStatus < 0) {
        status = NOTIFY_E_FAIL;
    }

    return status;
}


/** ----------------------------------------------------------------------------
 *  @name   NotifyPing_Callback
 *
 *  @desc   Callback
 *
 *  @modif  None.
 *  ----------------------------------------------------------------------------
 */
Void SlpmTransport_Callback (IN     Processor_Id procId,
    IN     UInt32       eventNo,
    IN OPT Void *       arg,
    IN OPT UInt32       payload)
{
    (Void) eventNo;
    (Void) procId ;
    (Void) payload;
    struct ping_arg eventClbkArg;

    eventClbkArg.arg1 = ((struct ping_arg  *)arg)->arg1;
    eventClbkArg.arg2 = ((struct ping_arg  *)arg)->arg2;

    Osal_printf ("------Called callbackfunction------\n" );

    if (eventNo == 19)
    {
        /* Get the user virtual address of the buffer */
        traceinfo.src  = 0x9cff0000;
        traceinfo.size = sizeof(sms);
        Memory_map (&traceinfo);
        /* Pointing to the share memory to read CCB */
        CCBStruct = (sms *)traceinfo.dst;

        /* Check if we received the CCB correctly */
        Osal_printf ("CCBpayload = 0x%x\n", payload);
        ccbPayload.whole = payload;
        Osal_printf ("CCBstuct->CAT=%d\n", CCBStruct->CAT);
        Osal_printf ("CCBstuct->ccb[%d].sub_type=%d\n",
                     ccbPayload.fields.ccb_num,
                     CCBStruct->ccb[ccbPayload.fields.ccb_num].sub_type);
        Osal_printf ("CCBstuct->ccb[%d].msg_type=%d\n",
                     ccbPayload.fields.ccb_num,
                     CCBStruct->ccb[ccbPayload.fields.ccb_num].msg_type);

        /* Prepare the ACK for Remote Proc (DUCATI) */
        ccbPayload.fields.reply_flag = TRUE;
        payload = ccbPayload.whole;

        /* Send event PRCM Ack (19) */
        Notify_sendEvent (notifyAppHandle,
                          MultiProc_getId("AppM3"),
                          eventNo,
                          payload,
                          FALSE);

        Osal_printf ("[NotifyPingExecute]>>>>Sent PRCM ACK to Ducati [%d]\n",
            eventNo);
    }
    else
    {
        NotifyPing_PostSem ((PVOID)eventClbkArg.arg1);
    }
}


static Int SlpmTransport_Execute (Void)
{
    Int                     status = NOTIFY_SUCCESS;
    /* Currently not used*/
    /* NotifyShmDrv_Attrs   driverAttrs; */
    /* UInt32               iter; */
    NotifyDriverShm_Params  params;
    UInt32                  payload;
    UInt32                  count = 0;
    /* Void               * flags = NULL; */
    UInt32                  fileId;
    UInt32                  entryPoint = 0;
    ProcMgr_StartParams     startParams;
    char *                  image_path;
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config sysCfg;
#else
    Notify_Config           config;
    ProcMgr_Params          procMgrParams;
    NotifyDriverShm_Config  drvConfig;
    ProcMgr_Config          cfg;
    MultiProc_Config        multiProcConfig;
    OMAP4430PROC_Config     procConfig;
    OMAP4430PROC_Params     procParams;
    ProcMgr_AttachParams    ducatiParams;
    ProcMgr_StopParams      stopParams;
#endif

    procId = 3;

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&sysCfg);
    status = SysMgr_setup (&sysCfg);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
        return status;
    }
    status = ProcMgr_open (&procHandle,
                           MultiProc_getId("SysM3"));
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        return status;
    }
#else
    UsrUtilsDrv_setup ();
    MultiProc_getConfig (&multiProcConfig);
    status = MultiProc_setup (&multiProcConfig);
    if (status < 0) {
        Osal_printf ("Error in MultiProc_setup [0x%x]\n", status);
        return status;
    }

    ProcMgr_getConfig (&cfg);
    status = ProcMgr_setup (&cfg);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_setup [0x%x]\n", status);
        return status;
    }

    OMAP4430PROC_getConfig (&procConfig);
    status = OMAP4430PROC_setup (&procConfig);
    if (status < 0) {
        Osal_printf ("Error in OMAP4430PROC_setup [0x%x]\n", status);
        return status;
    }

    OMAP4430PROC_Params_init (NULL, &procParams);
    procParams.numMemEntries       = NUM_MEM_ENTRIES;
    procParams.memEntries          = 0;
    procParams.resetVectorMemEntry = RESET_VECTOR_ENTRY_ID;
    proc4430Handle = OMAP4430PROC_create (procId, &procParams);
    if (proc4430Handle == NULL) {
        Osal_printf ("Error in OMAP4430PROC_create\n");
        return NOTIFY_E_FAIL;
    }

    ProcMgr_Params_init (NULL, &procMgrParams);
    procMgrParams.procHandle = proc4430Handle;
    procHandle = ProcMgr_create (2, &procMgrParams);
    if (procHandle == NULL) {
        Osal_printf ("Error in ProcMgr_create\n");
        return NOTIFY_E_FAIL;
    }

    status = ProcMgr_attach (procHandle,&ducatiParams);
    if (status < 0) {
        Osal_printf ("ProcMgr_attach status %d\n",status);
        return NOTIFY_E_FAIL;
    }

    Notify_getConfig (&config);
    Osal_printf ("Notify_getConfig Done\n");

    status = Notify_setup (&config);
    Osal_printf ("Notify_setup Done\n");
    if (status != NOTIFY_SUCCESS) {
        Osal_printf ("Error in Notify_setup [0x%x]\n", status);
        return status;
    }

    NotifyDriverShm_getConfig (&drvConfig);
    Osal_printf ("NotifyDriverShm_getConfig Done\n");
    status = NotifyDriverShm_setup (&drvConfig);
    if (status < 0) {
        Osal_printf ("Error in NotifyDriverShm_setup [0x%x]\n", status);
        return status;
    }
    else {
        Osal_printf ("NotifyDriverShm_setup Done\n");
    }
#endif

    /* Loading the notify SYSM3 image */
    startParams.proc_id = 2;
    image_path = NOTIFY_SYSM3_IMAGE_PATH;
    status = ProcMgr_load (procHandle, NOTIFY_SYSM3_IMAGE_PATH,
                           2, &image_path, &entryPoint,
                           &fileId, startParams.proc_id);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_load [0x%x]:SYSM3\n", status);
        return status;
    }

    status = ProcMgr_start (procHandle, 0, &startParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_start [0x%x]:SYSM3\n", status);
        return status;
    }

    /* Loading the PM APPM3 image */
    startParams.proc_id = 3;
    image_path = NOTIFY_APPM3_IMAGE_PATH;
    status = ProcMgr_load (procHandle, NOTIFY_APPM3_IMAGE_PATH,
                           3, &image_path, &entryPoint,
                           &fileId, startParams.proc_id);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_load [0x%x]:APPM3\n", status);
        return status;
    }

    status = ProcMgr_start (procHandle, 0, &startParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_start [0x%x]:APPM3\n", status);
        return status;
    }

    NotifyDriverShm_Params_init (NULL, &params);
    Osal_printf ("NotifyDriverShm_Params_init Done\n");
    /* Currently not used */
    /*params.sharedAddrSize     = NOTIFYAPP_SH_SIZE;*/
    params.numEvents          = 32;
    params.numReservedEvents  = 0;
    params.sendEventPollCount = (UInt32) -1;
    params.recvIntId          = NOTIFYAPP_RECV_INT_ID;
    params.sendIntId          = NOTIFYAPP_SEND_INT_ID;
    params.remoteProcId       = 3; /*PROC_DUCATI*/

    /* Create shared region */
    notifyAppHandle = NotifyDriverShm_create ("NOTIFYDRIVER_DUCATI", &params);
    if (notifyAppHandle == NULL) {
        Osal_printf ("Error in NotifyDriverShm_create [0x%x]\n", status);
        return NOTIFY_E_FAIL;
    }
    Osal_printf ("NotifyDriverShm_create Done\n");

    /* Registering the events 16 19 31 */

    eventNo = 16;

    /* Create Semaphore for the event */
    Osal_printf ("Create sem for event number %d\n", eventNo);
    status = NotifyPing_CreateSem (&event [eventNo]);
    if (status < 0) {
        Osal_printf ("Error in SlpmTransport_CreateSem [0x%x]\n", status);
        return status;
    }
    NotifyApp_recvEventCount [eventNo] = numIterations;

    /* Fill the callback args */
    evt_cbk_arg[eventNo].arg1 = (UInt32)event[eventNo];
    evt_cbk_arg[eventNo].arg2 = eventNo;

    /* Register Event */
    Osal_printf ("Registering for event number %d\n",eventNo);
    status = Notify_registerEvent (notifyAppHandle,
                                   procId,
                                   eventNo,
                                   (Notify_CallbackFxn)SlpmTransport_Callback,
                                   (Void *)&evt_cbk_arg[eventNo]);
    if (status < 0) {
        Osal_printf ("Error in Notify_registerEvent %d [0x%x]\n", eventNo,
                    status);
        return status;
    }
    Osal_printf ("Registered event number %d with Notify module\n",
                 eventNo);

    eventNo = 19;

    /* Create Semaphore for the event */
    Osal_printf ("Create sem for event number %d\n", eventNo);
    status = NotifyPing_CreateSem (&event [eventNo]);
    if (status < 0) {
        Osal_printf ("Error in SlpmTransport_CreateSem [0x%x]\n", status);
        return status;
    }
    NotifyApp_recvEventCount [eventNo] = numIterations;

    /* Fill the callback args */
    evt_cbk_arg[eventNo].arg1 = (UInt32)event[eventNo];
    evt_cbk_arg[eventNo].arg2 = eventNo;

    /* Register Event */
    Osal_printf ("Registering for event number %d\n",eventNo);
    status = Notify_registerEvent (notifyAppHandle,
                                   procId,
                                   eventNo,
                                   (Notify_CallbackFxn)SlpmTransport_Callback,
                                   (Void *)&evt_cbk_arg[eventNo]);
    if (status < 0) {
        Osal_printf ("Error in Notify_registerEvent %d [0x%x]\n", eventNo,
                    status);
        return status;
    }
    Osal_printf ("Registered event number %d with Notify module\n",
                 eventNo);

    eventNo = 31;

    /* Create Semaphore for the event */
    Osal_printf ("Create sem for event number %d\n", eventNo);
    status = NotifyPing_CreateSem (&event [eventNo]);
    if (status < 0) {
        Osal_printf ("Error in SlpmTransport_CreateSem [0x%x]\n", status);
        return status;
    }
    NotifyApp_recvEventCount [eventNo] = numIterations;

    /* Fill the callback args */
    evt_cbk_arg[eventNo].arg1 = (UInt32)event[eventNo];
    evt_cbk_arg[eventNo].arg2 = eventNo;

    /* Register Event */
    Osal_printf ("Registering for event number %d\n",eventNo);
    status = Notify_registerEvent (notifyAppHandle,
                                   procId,
                                   eventNo,
                                   (Notify_CallbackFxn)SlpmTransport_Callback,
                                   (Void *)&evt_cbk_arg[eventNo]);
    if (status < 0) {
        Osal_printf ("Error in Notify_registerEvent %d [0x%x]\n", eventNo,
                    status);
        return status;
    }
    Osal_printf ("Registered event number %d with Notify module\n",
                 eventNo);

    payload = 0xDEED1;
    status = NOTIFY_E_FAIL;

    /* Send event Play (16) */
    eventNo = 16;

    NotifyApp_recvEventCount [eventNo] = count;
    count = 0;

    payload++;
    Osal_printf ("------------Iteration\t %d ------------\n",
                    (UInt32)count);
    do
    {
        status = Notify_sendEvent (notifyAppHandle,
                                   procId,
                                   eventNo,
                                   payload,
                                   FALSE);
        if (status == NOTIFY_E_DRIVERINIT || status == NOTIFY_E_NOTREADY) {
            Osal_printf ("[SlpmTransportExecute] Notify Ducati-side Driver not"
                        "initialized yet !! status = 0x%x\n", status);
        }
    } while (status == NOTIFY_E_FAIL);

    Osal_printf ("Notify_sendEvent status [%d]\n",status);
    Osal_printf ("[SlpmTransportExecute]>>>>>>>> Sent Event [%d]\n",
        eventNo);
    Osal_printf ("[SlpmTransportExecute] Waiting on event %d\n", eventNo);
    NotifyPing_WaitSem (event [eventNo]);
    Osal_printf ("SlpmTransport_WaitSem status[%d] for Event[%d]\n",
                                                    status, eventNo);
    Osal_printf ("[SlpmTransportExecute]<<<<<<<< Received Event [%d]\n",
                                                            eventNo);
    NotifyApp_recvEventCount [eventNo] = count;

    /* Send event Stop/Exit (31) */
    eventNo = 31;

    NotifyApp_recvEventCount [eventNo] = count;
    count = 0;

    payload++;
    Osal_printf ("------------Iteration\t %d ------------\n",
                    (UInt32)count);
    status = Notify_sendEvent (notifyAppHandle,
                               procId,
                               eventNo,
                               payload,
                               FALSE);

    Osal_printf ("Notify_sendEvent status [%d]\n",status);
    Osal_printf ("[SlpmTransportExecute]>>>>>>>> Sent Event [%d]\n",
        eventNo);
    Osal_printf ("[SlpmTransportExecute] Waiting on event %d\n", eventNo);
    NotifyPing_WaitSem (event [eventNo]);
    Osal_printf ("SlpmTransport_WaitSem status[%d] for Event[%d]\n",
                                                    status, eventNo);
    Osal_printf ("[SlpmTransportExecute]<<<<<<<< Received Event [%d]\n",
                                                            eventNo);
    NotifyApp_recvEventCount [eventNo] = count;


    if (status < 0) {
        Osal_printf ("Error in Notify_sendEvent [0x%x]\n", status);
    }
    else {
        Osal_printf ("Sent %i events to event ID to remote processor\n",
                    numIterations);
    }

    Osal_printf ("Exit Ping Execute\n");
    return status;
}


Int SlpmTransport_shutdown (Void)
{
    Int32               status = 0;
    ProcMgr_StopParams  stopParams;

    Osal_printf ("Entered NotifyApp_shutdown\n");

    /* Unregister events. */
    eventNo = 16;
    status = Notify_unregisterEvent (notifyAppHandle,
                                     procId,
                                     eventNo,
                                     (Notify_CallbackFxn)SlpmTransport_Callback,
                                     &evt_cbk_arg[eventNo]);

    Osal_printf ("Notify_unregisterEvent status: [0x%x]\n", status);
    NotifyPing_DestroySem (event[eventNo]);
    eventNo = 19;
    status = Notify_unregisterEvent (notifyAppHandle,
                                     procId,
                                     eventNo,
                                     (Notify_CallbackFxn)SlpmTransport_Callback,
                                     &evt_cbk_arg[eventNo]);

    Osal_printf ("Notify_unregisterEvent status: [0x%x]\n", status);
    NotifyPing_DestroySem (event[eventNo]);
    eventNo = 31;
    status = Notify_unregisterEvent (notifyAppHandle,
                                     procId,
                                     eventNo,
                                     (Notify_CallbackFxn)SlpmTransport_Callback,
                                     &evt_cbk_arg[eventNo]);

    Osal_printf ("Notify_unregisterEvent status: [0x%x]\n", status);
    NotifyPing_DestroySem (event[eventNo]);

    status = NotifyDriverShm_delete (&(notifyAppHandle));
    if (status < 0) {
        Osal_printf ("NotifyDriverShm_delete status: [0x%x]\n", status);
        return status;
    }

    /* Call proc_stop, proc-detach, procmgr_delete,procmgr destroy*/
    stopParams.proc_id = MultiProc_getId ("SysM3");
    status = ProcMgr_stop (procHandle, &stopParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_stop [0x%d]:SYSM3\n", status);
        return status;
    }
    if (eventNo > 15) {
        stopParams.proc_id = MultiProc_getId ("AppM3");
        status = ProcMgr_stop (procHandle, &stopParams);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_stop [0x%d]:APPM3\n", status);
            return status;
        }
    }

#if !defined(SYSLINK_USE_SYSMGR)
    status =  ProcMgr_detach (procHandle);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_detach [0x%d]\n", status);
        return status;
    }
    status = ProcMgr_delete (&procHandle);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_delete [0x%d]\n", status);
        return status;
    }

    status = ProcMgr_destroy ();
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_destroy [0x%d]\n", status);
        return status;
    }

    /* Call Omap4430 delete() and destroy()*/
    status = OMAP4430PROC_delete (&proc4430Handle);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_destroy [0x%d]\n", status);
        return status;
    }

    OMAP4430PROC_destroy ();
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_destroy [0x%d]\n", status);
        return status;
    }
#endif

#if !defined(SYSLINK_USE_SYSMGR)
    status = NotifyDriverShm_destroy ();
    if (status < 0) {
        Osal_printf ("NotifyDriverShm_destroy status: [0x%x]\n", status);
        return status;
    }

    status = Notify_destroy ();
    if (status < 0) {
        Osal_printf ("Notify_destroy status: [0x%x]\n", status);
        return status;
    }
    status = MultiProc_destroy();
    if (status < 0) {
        Osal_printf ("MultiProc_destroy status: [0x%x]\n", status);
        return status;
    }
#else
    status = ProcMgr_close (&procHandle);
    if (status < 0) {
        Osal_printf ("ProcMgr_close  status: [0x%x]\n", status);
    }
    SysMgr_destroy ();
#endif

    Osal_printf ("Leaving SlpmTransport_shutdown\n");

    return 0;
}



/** ============================================================================
 *  @func   main()
 *
 *  ============================================================================
 */
Int main (Int argc, Char * argv [])
{
    Int status = NOTIFY_SUCCESS;

    if (argc == 1) {
        printf ("------PM SAMPLE---------\n");
        printf ("Simulating a Play command:\n");
        printf ("Setting Event 16 as play\n");
        printf ("Setting Event 31 as stop/exit\n");
        printf ("Setting Event 19 as PRCM ack\n");

        status = SlpmTransport_Execute ();
        if (status < 0) {
            Osal_printf ("Error in SlpmTransport_Execute %d \n",status);
        }
        else {
            status = SlpmTransport_shutdown ();
        }
    }

    return 0;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
