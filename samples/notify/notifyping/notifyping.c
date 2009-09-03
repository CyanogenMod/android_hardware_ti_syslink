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
#include <string.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

#define NOTIFY_DUCATI_EVENTNUMBER 0x00000001


/** ============================================================================
 *  @macro  EVENT_STARTNO
 *
 *  @desc   Number after resevred events.
 *  ============================================================================
 */
#define  EVENT_STARTNO  3

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          receive events.
 */
#define NOTIFYAPP_RECV_INT_ID     26

/*!
 *  @brief  Interrupt ID of physical interrupt handled by the Notify driver to
 *          send events.
 */
#define NOTIFYAPP_SEND_INT_ID     55

#define  NOTIFYAPP_NUMEVENTS      5


UInt16 NotifyApp_recvEventCount [NOTIFYAPP_NUMEVENTS];

#define NOTIFYAPP_EVENT_NO 3

#define NOTIFYAPP_NUM_TRANSFERS 100

#define MAX_EVENTS	32


/** ============================================================================
 *  @macro  numIterations
 *
 *  @desc   Number of ietrations.
 *  ============================================================================
 */
Uint16 numIterations = NOTIFYAPP_NUMEVENTS;

Processor_Id procId;

PVOID	event [MAX_EVENTS];
Uint16 	eventNo;


typedef struct notify_ping_semobj_tag {
    sem_t  sem ;
} notify_ping_semobj ;

struct ping_arg{
	Uint32 arg1;
	Uint32 arg2;
};

struct ping_arg evt_cbk_arg[MAX_EVENTS];

NotifyDriverShm_Handle NotifyApp_handle = NULL;

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
static Void NotifyPing_Execute (Void);
int NotifyApp_shutdown (Void);

int NotifyPing_CreateSem (OUT PVOID * semPtr)
{
    Int status = NOTIFY_SUCCESS ;
    notify_ping_semobj * semObj ;
    int                  osStatus ;

    semObj = malloc (sizeof (notify_ping_semobj)) ;
    if (semObj != NULL) {
        osStatus = sem_init (&(semObj->sem), 0, 0) ;
        if (osStatus < 0) {
            status = DSP_EFAIL ;
        }
        else {
            *semPtr = (PVOID) semObj ;
        }
    }
    else {
        *semPtr = NULL ;
        status = DSP_EFAIL ;
    }

    return status ;
}



int  NotifyPing_PostSem (IN PVOID semHandle)
{
	Int  status = NOTIFY_SUCCESS ;
	notify_ping_semobj * semObj = semHandle ;
	int                  osStatus ;

	osStatus = sem_post (&(semObj->sem)) ;
	if (osStatus < 0) {
		status = DSP_EFAIL ;
	}

	return status ;
}


int NotifyPing_WaitSem (IN PVOID semHandle)
{
	Int           status = NOTIFY_SUCCESS;
	notify_ping_semobj * semObj = semHandle ;
	int                  osStatus ;
	printf("NotifyPing_WaitSem\n");
	osStatus = sem_wait (&(semObj->sem)) ;
	if (osStatus < 0) {
		status = NOTIFY_E_FAIL ;
	}
	return status ;
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
	IN     Uint32       eventNo,
	IN OPT Void *       arg,
	IN OPT Uint32       payload)
{
	(Void) eventNo ;
	(Void) procId  ;
	(Void) payload ;
	struct ping_arg event_clbk_arg;

	event_clbk_arg.arg1 = ((struct ping_arg  *)arg)->arg1;
	event_clbk_arg.arg2 = ((struct ping_arg  *)arg)->arg2;

	printf("------Called callbackfunction------\n" );

	NotifyPing_PostSem ((PVOID)event_clbk_arg.arg1) ;
}


static Void NotifyPing_Execute (Void)
{
	Int  status = NOTIFY_SUCCESS;
	Notify_Config        config ;
	/* Currently not used*/
	/*	NotifyShmDrv_Attrs   driverAttrs ;*/
	/*Uint32               iter ;*/
	NotifyDriverShm_Config  drvConfig;
	NotifyDriverShm_Params  params;

	Uint32 		payload;
	Uint32 count = 0;
	/*void * flags = NULL;*/
	int i;
	int j;
	Uint16 event_id;

	procId = 3;


	if(eventNo == MAX_EVENTS)
	{
		event_id = MAX_EVENTS-1;
	} else {
		event_id = eventNo;
	}

	printf("Calling getconfig\n");

	Notify_getConfig (&config);

	printf("Done calling get config\n");

	status = Notify_setup (&config);

	printf("Done calling Set_up\n");

	if (status != NOTIFY_SUCCESS) {
		printf ("Notify_setup failed! Status [0x%x]\n",
		status) ;
		goto func_end;

	}

	printf("Calling NotifyDriverShm_getConfig\n");

	NotifyDriverShm_getConfig (&drvConfig);

	printf("Calling NotifyDriverShm_setup\n");
	status = NotifyDriverShm_setup (&drvConfig);
	if (status < 0) {
		printf ("Error in NotifyDriverShm_setup [0x%x]\n", status);
		status = NOTIFY_E_FAIL;
	}else {
		status = NOTIFY_SUCCESS;
		printf ("NotifyDriverShm_setup Pass\n");
	}


	if (status != NOTIFY_SUCCESS) {
		printf("Setup failed with status %d\n", status);
		goto func_end;
	}
	printf("Calling NotifyDriverShm_Params_init\n");
	NotifyDriverShm_Params_init (NULL, &params);
	/* Currently not used*/
	/*params.sharedAddrSize     = NOTIFYAPP_SH_SIZE;*/
	params.numEvents          = 32;
	params.numReservedEvents  = 0;
	params.sendEventPollCount = (UInt32) -1;
	params.recvIntId          = NOTIFYAPP_RECV_INT_ID;
	params.sendIntId          = NOTIFYAPP_SEND_INT_ID;
	params.remoteProcId       = 3; /*PROC_DUCATI*/

	printf("Calling NotifyDriverShm_create\n");

	/* Create shared region */
	NotifyApp_handle = NotifyDriverShm_create \
				("NOTIFYDRIVER_DUCATI", &params);
	printf ("NotifyDriverShm_create Handle: [0x%x]\n",
			(unsigned int )NotifyApp_handle);

	if (NotifyApp_handle == NULL) {
		printf ("Error in NotifyDriverShm_create\n");
		status = NOTIFY_E_FAIL;
	}

	/* Register for all events */
	if(eventNo == MAX_EVENTS) {
		printf("Registering for all events\n");

		for(i=EVENT_STARTNO;i<MAX_EVENTS;i++) {

			/* Create the semaphore */
			NotifyPing_CreateSem (&event [i]) ;

			/*Register for the event */
			NotifyApp_recvEventCount [i] = numIterations;
			evt_cbk_arg[i].arg1 = (Uint32)event[i];
			evt_cbk_arg[i].arg2 = i;

			status = Notify_registerEvent (NotifyApp_handle,
							procId,
							i,
							(Notify_CallbackFxn)NotifyPing_Callback,
							(Void *)&evt_cbk_arg[i]);

			if (status < 0) {
				printf ("Error in Notify_registerEvent %d"
					" [0x%x]\n", i, status);
				/* FIXME : Exit with error status */
			}
			/*TBD :: If following prints are needed. */
			else {
				printf ("Registered event number %d with"
						" Notify module\n",
						i);
			}
		}
	}	else {
		/* Create Semaphore for the event */
		printf("Create sem for event number %d\n",eventNo);
		NotifyPing_CreateSem (&event [eventNo]) ;



		NotifyApp_recvEventCount [eventNo] = numIterations;

		/* Fill the callback args */
		evt_cbk_arg[eventNo].arg1 = (Uint32)event[eventNo];
		evt_cbk_arg[eventNo].arg2 = eventNo;

		/* Register Event */
		printf("Registering for event number %d\n",eventNo);
		status = Notify_registerEvent (NotifyApp_handle,
					procId,
					eventNo,
				(Notify_CallbackFxn)NotifyPing_Callback,
				(Void *)&evt_cbk_arg[eventNo]);

		if (status < 0) {
			printf ("Error in Notify_registerEvent %d"
				" [0x%x]\n", eventNo, status);
		} else {
			printf ("Registered event number %d with"
					" Notify module\n",
					eventNo);
		}

	}


	payload = 0xDEED1;


	/* Keep sending events until ducati side is up and ready to receive
	 events */
	printf("Calling Notify_sendEvent for event_id %d\n",event_id);
	status = Notify_sendEvent (NotifyApp_handle,
				procId, event_id, 0, TRUE);

	do {
		printf("-->In do while loop and status is %x eventno %d \n",
			status, event_id);

		status = Notify_sendEvent (NotifyApp_handle,
				procId,event_id,
				payload, FALSE) ;
		if(status == NOTIFY_E_DRIVERINIT ||
			status == NOTIFY_E_NOTREADY)
			printf("[NotifyPingExecute] Notify Ducati-side \
				Driver not initialized yet !! status \
				= 0x%x\n", status);

	} while (status == NOTIFY_E_FAIL);

	printf("[NotifyPingExecute]>>>>>>>> Sent Event [1]\
		\n");

	if (status != NOTIFY_SUCCESS) {
		printf("Error after send event, status %d\n", status);
		goto func_end;

	}

	/* Start sending and receiving events */
	if(eventNo == MAX_EVENTS) {

		/* Send all events, from 3-32, to Ducati for each
		   number of transfer */
		for(j= (numIterations-1); j>=0;j--) {

			/* Send for events from 4- 32 */
			for(i=EVENT_STARTNO+1;i<MAX_EVENTS;i++) {

				payload++;
				status = Notify_sendEvent (NotifyApp_handle,
							procId,
							i,
							payload, FALSE) ;
				printf("[NotifyPingExecute]>>>>>>>> Sent Event\
					[x]\n");

			}

			/* Wait for events from 4-32 */
			for(i=EVENT_STARTNO;i<MAX_EVENTS;i++) {
				NotifyPing_WaitSem (event [i]) ;
				printf("[NotifyPingExecute]<<<<<<<< Received \
					Event []\n");
			}
			NotifyApp_recvEventCount [eventNo] = j;


			/* Stop iterations here */
			if(j==0 )
				break;
			/* Start the next iteration here */

			payload++;

			/* Send event 3 for the next iteration */
			status = Notify_sendEvent (NotifyApp_handle,
						procId,
						EVENT_STARTNO,
						payload, FALSE) ;
			printf("[NotifyPingExecute]>>>>>>>> Sent Event\
				[x]\n");


		}

	}else {

		count = numIterations-1;
		while(count--) {

			printf("[NotifyPingExecute] Waiting on event %d\n",
				eventNo);
			NotifyPing_WaitSem (event [eventNo]) ;
			/*FIXME: put receive count here */
			printf("[NotifyPingExecute]<<<<<<<< Received \
				Event []\n");

			NotifyApp_recvEventCount [eventNo] = count;


			payload++;

			printf("[NotifyPingExecute] SendEvent \
				[0x%x 0x%x, 0x%x, 0x%x]\n",
				(unsigned int )NotifyApp_handle,
				(unsigned int )procId,
				(unsigned int )event[eventNo],
				(unsigned int )payload);

			status = Notify_sendEvent (NotifyApp_handle,
						procId,
						eventNo,
						payload, FALSE) ;
			printf("[NotifyPingExecute]>>>>>>>> Sent Event\
				[x]\n");
		}
		printf("[NotifyPingExecute] WAITING FOR LAST EVENT\n");
		NotifyPing_WaitSem (event [eventNo]) ;
		printf("[NotifyPingExecute]<<<<<<<< Received LAST \
			Event []\n");
	}

	if (status < 0) {
		printf ("Error in Notify_sendEvent [0x%x]\n",
			 status);
	}
	else {
		printf ("Sent %i events to event ID to "
				"remote processor\n",numIterations);
	}



func_end:
	printf("Exit Ping Execute\n");
}

Int NotifyApp_shutdown (Void)
{
	Int32  status = 0;
	UInt32 i;

	printf ("Entered NotifyApp_shutdown\n");

	/* Unregister event. */
	if(eventNo == MAX_EVENTS) {
		for(i=EVENT_STARTNO;i<MAX_EVENTS;i++) {
			status = Notify_unregisterEvent (NotifyApp_handle,
							procId, i,
						(Notify_CallbackFxn)NotifyPing_Callback,
							&evt_cbk_arg[i]);
		}
	} else {
	status = Notify_unregisterEvent (NotifyApp_handle,
	                                 procId,
	                                 eventNo,
	                                 (Notify_CallbackFxn)NotifyPing_Callback,
	                                 &evt_cbk_arg[eventNo]);
	}
	printf ("Notify_unregisterEvent status: [0x%x]\n", status);

	status = NotifyDriverShm_delete (&(NotifyApp_handle));
	printf ("NotifyDriverShm_delete status: [0x%x]\n", status);

	status = NotifyDriverShm_destroy ();
	printf ("NotifyDriverShm_destroy status: [0x%x]\n", status);

	status = Notify_destroy ();
	printf ("Notify_destroy status: [0x%x]\n", status);

	printf ("ProcMgr_close status: [0x%x]\n", status);

	printf ("Leaving NotifyApp_shutdown\n");

	return 0;
}



/** ============================================================================
 *  @func   main()
 *
 *  ============================================================================
 */
int
main (int argc, char * argv [])
{
	if(argc == 1) {
		int choice;
		printf("------NOTIFY PING SAMPLE---------\n");
		printf("Options:\n");
		printf("1. Execute default configuration (Iteration (5) Event (3) )\n");
		printf("2. Enter Iteration value (>1) and event no (3-31))\n");
		printf("3. Execute Iterations (5) for all events\n");
		printf("Enter choice: ");

		scanf("%d",&choice);

		switch(choice) {
			case 2:
				printf("\nIterations(>1): ");
				scanf("%hu",&numIterations);
				printf("\nEvent No(3-31): ");
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
		NotifyPing_Execute ();
		NotifyApp_shutdown ();
	}

	if(argc == 2) {
		printf("---Using defult event number as 3--");
		eventNo = EVENT_STARTNO;
		numIterations = NOTIFYXFER_Atoi (argv [1]) ;

		printf("Calling NOTIFYPing_execute\n for %d iterations and\n \
			Event number %d\n",numIterations, eventNo);
		NotifyPing_Execute ();
		NotifyApp_shutdown ();
	}

	if (argc == 3) {
		eventNo	= NOTIFYXFER_Atoi (argv [2]) ;
		if(eventNo == MAX_EVENTS) {
			printf(" Notifyping will run for all events\n");
		}
		else if(eventNo > MAX_EVENTS || eventNo < EVENT_STARTNO )
		{
			printf("Event number should be in the range 3-32 \n");
			return 0;
		}
		numIterations = NOTIFYXFER_Atoi (argv [1]) ;
		printf("Calling NOTIFYPing_execute\n for %d iterations and\n \
			Event number %d\n",numIterations, eventNo);
		NotifyPing_Execute ();
		NotifyApp_shutdown ();
	}
	return 0 ;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
