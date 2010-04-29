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

#define NOTIFYAPP_NUMEVENTS     5

#define NOTIFYAPP_EVENT_NO      3

#define NOTIFYAPP_NUM_TRANSFERS 100

#define MAX_EVENTS              32

#define NUM_MEM_ENTRIES         9

#define RESET_VECTOR_ENTRY_ID   0

#define NOTIFY_SYSM3_IMAGE_PATH "./Notify_MPUSYS_Test_Core0.xem3"
#define NOTIFY_APPM3_IMAGE_PATH "./Notify_MPUAPP_Test_Core1.xem3"

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
static Int NotifyPing_Execute (Void);
Int NotifyApp_shutdown (Void);

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
Void NotifyPing_Callback (IN     Processor_Id procId,
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

    NotifyPing_PostSem ((PVOID)eventClbkArg.arg1);
}


static Int NotifyPing_Execute (Void)
{
    Int                     status = NOTIFY_SUCCESS;
    /* Currently not used*/
    /* NotifyShmDrv_Attrs   driverAttrs; */
    /* UInt32               iter; */
    NotifyDriverShm_Params  params;
    UInt32                  payload;
    UInt32                  count = 0;
    /* Void               * flags = NULL; */
    Int                     i;
    Int                     j;
    UInt16                  eventId;
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
    if (eventNo == MAX_EVENTS) {
        eventId = MAX_EVENTS - 1;
    }
    else {
        eventId = eventNo;
    }

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
        Osal_printf("ProcMgr_attach status %d\n",status);
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

    if (eventNo > 15) {
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

    /* Register for all events */
    if (eventNo == MAX_EVENTS) {
        Osal_printf ("Registering for all events\n");

        for(i = EVENT_STARTNO; i < MAX_EVENTS; i++) {
            /* Create the semaphore */
            status = NotifyPing_CreateSem (&event[i]);
            if (status < 0) {
                Osal_printf ("Error in NotifyPing_CreateSem [0x%x]\n", status);
                return status;
            }
            /*Register for the event */
            NotifyApp_recvEventCount [i] = numIterations;
            evt_cbk_arg[i].arg1 = (UInt32)event[i];
            evt_cbk_arg[i].arg2 = i;

            status = Notify_registerEvent (notifyAppHandle,
                            procId,
                            i,
                            (Notify_CallbackFxn)NotifyPing_Callback,
                            (Void *)&evt_cbk_arg[i]);
            if (status < 0) {
                Osal_printf ("Error in Notify_registerEvent status [%d] "
                             "Event[0x%x]\n", status, i);
                return status;
            }
            Osal_printf ("Registered event number %d with Notify module\n", i);
        }
    }
    else {
        /* Create Semaphore for the event */
        Osal_printf ("Create sem for event number %d\n", eventNo);
        status = NotifyPing_CreateSem (&event [eventNo]);
        if (status < 0) {
            Osal_printf ("Error in NotifyPing_CreateSem [0x%x]\n", status);
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
                                       (Notify_CallbackFxn)NotifyPing_Callback,
                                       (Void *)&evt_cbk_arg[eventNo]);
        if (status < 0) {
            Osal_printf ("Error in Notify_registerEvent %d [0x%x]\n", eventNo,
                        status);
            return status;
        }
        Osal_printf ("Registered event number %d with Notify module\n",
                     eventNo);
    }

    payload = 0xDEED1;
    status = NOTIFY_E_FAIL;

    /* Keep sending events until ducati side is up and ready to receive
     events */

    do {
        status = Notify_sendEvent (notifyAppHandle,
                                   procId, eventId,
                                   payload, FALSE);
        if (status == NOTIFY_E_DRIVERINIT || status == NOTIFY_E_NOTREADY) {
            Osal_printf ("[NotifyPingExecute] Notify Ducati-side Driver not "
                    "initialized yet !! status = 0x%x\n", status);
        }
    } while (status == NOTIFY_E_FAIL);

    Osal_printf ("---------------------------------------------------------\n");
    Osal_printf ("-----------------[NotifyPingExecute]---------------------\n");
    Osal_printf ("---------------------------------------------------------\n");
    Osal_printf ("------------Iteration\t %d-------------\n", numIterations);

    Osal_printf ("[NotifyPingExecute]>>>>>>>> Sent Event [%d]\n", eventId);

    if (status != NOTIFY_SUCCESS) {
        Osal_printf ("Error after send event, status %d\n", status);
        return status;
    }

    /* Start sending and receiving events */
    if (eventNo == MAX_EVENTS) {
        /* Send all events, from 3-32, to Ducati for each
           number of transfer */
        for(j = (numIterations-1); j >= 0; j--) {
            /* Send for events from 4- 32 */
            for(i = EVENT_STARTNO + 1; i < MAX_EVENTS; i++) {
                payload++;
                status = Notify_sendEvent (notifyAppHandle,
                                           procId,
                                           i,
                                           payload,
                                           FALSE);
                Osal_printf ("Notify_sendEvent status[%d] for Event[%d]\n"
                                                                   ,status, i);
                Osal_printf ("[NotifyPingExecute]>>>>>>>> Sent Event[%d]\n", i);
            }

            /* Wait for events from 4-32 */
            for(i = EVENT_STARTNO + 1; i < MAX_EVENTS; i++) {
                Osal_printf ("[NotifyPingExecute] Waiting on event %d\n",
                             eventNo);
                status = NotifyPing_WaitSem (event [i]);
                Osal_printf ("NotifyPing_WaitSem status[%d] for Event[%d]\n",
                                                                   status, i);
                Osal_printf ("[NotifyPingExecute]<<<<<<<< Received Event[%d]\n",
                             i);
                NotifyApp_recvEventCount [i] = j;
            }

            /* Stop iterations here */
            if (j==0 )
                break;
            /* Start the next iteration here */

            payload++;

            /* Send event 3 for the next iteration */
            status = Notify_sendEvent (notifyAppHandle,
                                       procId,
                                       EVENT_STARTNO,
                                       payload,
                                       FALSE);
            Osal_printf ("Notify_sendEvent status[%d] for Event[3] \n", status);
            Osal_printf ("[NotifyPingExecute]>>>>>>>> Sent Event[3]\n");
        }
    }
    else {
        Osal_printf ("[NotifyPingExecute] Waiting on event %d\n", eventNo);
        status = NotifyPing_WaitSem (event [eventNo]);
        Osal_printf ("NotifyPing_WaitSem status[%d] for Event[%d]\n", status,
                                                                eventNo);
        /* FIXME: put receive count here */
        Osal_printf ("[NotifyPingExecute]<<<<<<<< Received Event [%d]\n",
                     eventNo);

        NotifyApp_recvEventCount [eventNo] = count;
        count = numIterations - 1;

        while (count) {
            payload++;
            Osal_printf ("------------Iteration\t %d ------------\n",
                            (UInt32)count);
            status = Notify_sendEvent(notifyAppHandle,
                                      procId,
                                      eventNo,
                                      payload,
                                      FALSE);

            Osal_printf ("Notify_sendEvent status [%d]\n",status);
            Osal_printf ("[NotifyPingExecute]>>>>>>>> Sent Event [%d]\n",
                eventNo);
            Osal_printf ("[NotifyPingExecute] Waiting on event %d\n", eventNo);
            NotifyPing_WaitSem (event [eventNo]);
            Osal_printf ("NotifyPing_WaitSem status[%d] for Event[%d]\n",
                                                            status, eventNo);
            Osal_printf ("[NotifyPingExecute]<<<<<<<< Received Event [%d]\n",
                                                                    eventNo);
            NotifyApp_recvEventCount [eventNo] = count;
            count--;
        }
    }

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


Int NotifyApp_shutdown (Void)
{
    Int32               status = 0;
    UInt32              i;
    ProcMgr_StopParams  stopParams;

    Osal_printf ("Entered NotifyApp_shutdown\n");
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

    /* Unregister event. */
    if (eventNo == MAX_EVENTS) {
        for(i = EVENT_STARTNO; i < MAX_EVENTS; i++) {
            status = Notify_unregisterEvent (notifyAppHandle, procId, i,
                                    (Notify_CallbackFxn)NotifyPing_Callback,
                                    &evt_cbk_arg[i]);

            Osal_printf ("Notify_unregisterEvent status: [0x%x] Event[%d]\n",
                                                                  status, i);
            NotifyPing_DestroySem (event[i]);
        }
    }
    else {
        status = Notify_unregisterEvent (notifyAppHandle,
                                     procId,
                                     eventNo,
                                     (Notify_CallbackFxn)NotifyPing_Callback,
                                     &evt_cbk_arg[eventNo]);

        Osal_printf ("Notify_unregisterEvent status: [0x%x]\n", status);
        NotifyPing_DestroySem (event[eventNo]);
    }

    status = NotifyDriverShm_delete (&(notifyAppHandle));
    if (status < 0) {
        Osal_printf ("NotifyDriverShm_delete status: [0x%x]\n", status);
        return status;
    }

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

    Osal_printf ("Leaving NotifyApp_shutdown\n");

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
        Int choice;
        printf ("------NOTIFY PING SAMPLE---------\n");
        printf ("Options:\n");
        printf ("1. Execute default configuration (Iteration (5) Event (3) )\n");
        printf ("2. Enter Iteration value (>1) and event no (3-31))\n");
        printf ("3. Execute Iterations (5) for all events\n");
        printf ("Enter choice: ");

        scanf("%d",&choice);

        switch(choice) {
            case 2:
                printf ("\nIterations(>1): ");
                scanf("%hu",&numIterations);
                printf ("\nEvent No(3-31): ");
                scanf("%hu",&eventNo);
                break;

            case 3:
                eventNo = MAX_EVENTS;
                numIterations = NOTIFYAPP_NUMEVENTS;
                break;

            case 1:
            default:
                eventNo = EVENT_STARTNO;
                numIterations = NOTIFYAPP_NUMEVENTS;
        }

        status = NotifyPing_Execute ();
        if (status < 0) {
            Osal_printf("Error in NotifyPing_Execute %d \n",status);
        }
        else {
            status = NotifyApp_shutdown ();
        }
    }

    if (argc == 2) {
        Osal_printf ("---Using defult event number as 3--");
        eventNo = EVENT_STARTNO;
        numIterations = NOTIFYXFER_Atoi (argv [1]);

        Osal_printf ("Starting NotifyPing_execute\n for %d iterations and\n \
                                Event number %d\n",numIterations, eventNo);

        status = NotifyPing_Execute ();
        if (status < 0) {
            Osal_printf("Error in NotifyPing_Execute %d \n",status);
        }
        else {
            status = NotifyApp_shutdown ();
        }
    }

    if (argc == 3) {
        eventNo    = NOTIFYXFER_Atoi (argv [2]);
        if (eventNo == MAX_EVENTS) {
            Osal_printf (" Notifyping will run for all events\n");
        }
        else if (eventNo > MAX_EVENTS || eventNo < EVENT_STARTNO )
        {
            Osal_printf ("Event number should be in the range 3-32 \n");
            return 0;
        }
        numIterations = NOTIFYXFER_Atoi (argv [1]);
        Osal_printf ("Calling NOTIFYPing_execute\n for %d iterations and\n \
            Event number %d\n",numIterations, eventNo);

        status = NotifyPing_Execute ();
        if (status < 0) {
            Osal_printf("Error in NotifyPing_Execute %d \n",status);
        }
        else {
            status = NotifyApp_shutdown ();
        }

    }

    return 0;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
