#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <dbapi.h>
#include <DSPManager.h>
#include <DSPProcessor.h>
#include <DSPProcessor_OEM.h>
#include <pthread.h>

/* #undef PALMS_BUILD */

#ifndef PALMS_BUILD
    #define PmLogPrintInfo(...)
    #define PmLogPrintError(...)
#else
   #include <PmLogLib/IncsPublic/PmLogLib.h>
   PmLogContext gTiDspMonLogContext;
#endif

/**************************
 ***   MACROS/DEFINES   ***
 **************************/

/** Options **/

/* Enabling this flag will enable traces */
#define APP_DEBUG

/** Host Specific **/
#define BASEIMAGE_FILE      "/lib/dsp/baseimage.dof"

/** Debugging **/
#ifdef APP_DEBUG
	#define DEBUG_PRINT(...) \
	{ \
	   fprintf(stderr, "DBG:%d:%s::", __LINE__, __FUNCTION__); \
	   fprintf(stderr, __VA_ARGS__);\
	   PmLogPrintInfo(gTiDspMonLogContext, "DBG:%d:%s::", __LINE__, __FUNCTION__); \
	   PmLogPrintInfo(gTiDspMonLogContext, __VA_ARGS__);\
	}

	#define ERROR_PRINT(...) \
	{ \
	   fprintf(stderr, "ERR:%d:%s::", __LINE__, __FUNCTION__); \
	   fprintf(stderr, __VA_ARGS__);\
	   PmLogPrintError(gTiDspMonLogContext, "ERR:%d:%s::", __LINE__, __FUNCTION__); \
	   PmLogPrintError(gTiDspMonLogContext, __VA_ARGS__);\
	}
#else
	#define DEBUG_PRINT(...)
	#define ERROR_PRINT(...)
#endif

#define LOG_INFO_PRINT(...) \
{ \
   fprintf(stderr, "LOG: %d:%s::", __LINE__, __FUNCTION__); \
   fprintf(stderr, __VA_ARGS__); \
   PmLogPrintInfo(gTiDspMonLogContext,__VA_ARGS__); \
}

#define LOG_ERROR_PRINT(...) \
{ \
   fprintf(stderr, "ERR: %d:%s::", __LINE__, __FUNCTION__); \
   fprintf(stderr, __VA_ARGS__); \
   PmLogPrintError(gTiDspMonLogContext,__VA_ARGS__); \
}

/** Helper Macros **/
#define DSP_ERROR_EXIT(err, msg, label)                  \
    if (DSP_FAILED(err)) {                               \
	ERROR_PRINT("%s : Err Num = %lx\n", msg, err);   \
	goto label;                                      \
    }

#define MALLOC(_var, _type)\
    _var = (_type *) malloc(sizeof(_type));              \
    if (_var == NULL) {                                  \
	DEBUG_PRINT("%d :: malloc failed\n", __LINE__);  \
	goto EXIT;                                       \
    }                                                    \
    memset(_var, 0, sizeof(_type));

/* Function Definitions */
DSP_STATUS Crash_Listener();
DSP_STATUS Attach_DspProc(DSP_HPROCESSOR *hProc);
DSP_STATUS Dettach_DspProc(DSP_HPROCESSOR hProc);
DSP_STATUS Recover_Dsp(DSP_HPROCESSOR *hProc);
DSP_STATUS Load_Baseimage(DSP_HPROCESSOR hProc);


int main(int argc, char *argv[]) {
    DSP_STATUS status = DSP_SOK;

#ifdef PALMS_BUILD
	PmLogErr err;

    err = PmLogGetContext("tidspmon", &gTiDspMonLogContext);

	switch(err)
	{
		case kPmLogErr_None:
			DEBUG_PRINT("PmLogGetContext returned: kPmLogErr_None");
			break;
		case kPmLogErr_InvalidParameter:
			DEBUG_PRINT("PmLogGetContext returned: kPmLogErr_InvalidParameter");
			break;

		case kPmLogErr_InvalidContext:
			DEBUG_PRINT("PmLogGetContext returned: kPmLogErr_InvalidContext");
			break;
		case kPmLogErr_InvalidContextName:
			DEBUG_PRINT("PmLogGetContext returned: kPmLogErr_InvalidContextName");
			break;
		default:
			DEBUG_PRINT("PmLogGetContext returned: %d\n", err);
			break;
	}

#endif

    status = Crash_Listener();

    DEBUG_PRINT("Stopped listening for DSP crashes\n");

    return (int) status;
}

DSP_STATUS Crash_Listener() {
    DSP_STATUS status = DSP_SOK;
    DSP_HPROCESSOR hProc;
    unsigned int index=0;
    struct DSP_NOTIFICATION* notificationObjects[4];
    struct DSP_PROCESSORSTATE ProcStatus;
    unsigned int maxretries = 10;

    MALLOC(notificationObjects[0], struct DSP_NOTIFICATION);
    MALLOC(notificationObjects[1], struct DSP_NOTIFICATION);
    MALLOC(notificationObjects[2], struct DSP_NOTIFICATION);
    MALLOC(notificationObjects[3], struct DSP_NOTIFICATION);

    status = DspManager_Open(0,NULL);
    DSP_ERROR_EXIT(status,"Couldn't Open DSP Manager",EXIT);

    LOG_INFO_PRINT("TI DSP Monitor Starting\n");

	status = Attach_DspProc(&hProc);
	DSP_ERROR_EXIT(status,"Couldn't attach to DSP",EXIT);

    do {
        DSPProcessor_GetState(hProc, &ProcStatus, sizeof(ProcStatus));
        DEBUG_PRINT("DSP Proc State = %d\n", ProcStatus.iState);

        if (ProcStatus.iState != PROC_RUNNING) {
            DEBUG_PRINT("DSP PROC state is not RUNNING, reloading baseimage\n");
            status = Load_Baseimage(hProc);
            sleep(1);
        }
    } while(ProcStatus.iState != PROC_RUNNING && maxretries-- > 0);

        status = DSPProcessor_RegisterNotify(hProc, DSP_SYSERROR, DSP_SIGNALEVENT, notificationObjects[0]);
        DSP_ERROR_EXIT(status, "DSP node register notify DSP_MMUFAULT", EXIT);
        status = DSPProcessor_RegisterNotify(hProc, DSP_MMUFAULT, DSP_SIGNALEVENT, notificationObjects[1]);
        DSP_ERROR_EXIT(status, "DSP node register notify DSP_SYSERROR", EXIT);
        status = DSPProcessor_RegisterNotify(hProc, DSP_PROCESSORSTATECHANGE, DSP_SIGNALEVENT, notificationObjects[2]);
        DSP_ERROR_EXIT(status, "DSP node register notify DSP_StateChange", EXIT);
        status = DSPProcessor_RegisterNotify(hProc, DSP_PWRERROR, DSP_SIGNALEVENT, notificationObjects[3]);
        DSP_ERROR_EXIT(status, "DSP node register notify DSP_PWRERROR", EXIT);


    while (1) {
        DEBUG_PRINT("Listening for DSP crashes...\n");
        status = DSPManager_WaitForEvents(notificationObjects, 4, &index, DSP_FOREVER);
        if (DSP_SUCCEEDED(status)) {
            DEBUG_PRINT("Dsp Crash detected (%s), trying to recover...\n",
                (index == 0 ? "MMU_FAULT" : (index == 1 ? "DSP_ERROR" : "STATE_CHANGE")));

            if (index == 0 || index == 1) {
                Dettach_DspProc(hProc);
                status = Recover_Dsp(&hProc);
                DSP_ERROR_EXIT(status, "Couldn't recover from DSP Error",EXIT);

				status = Attach_DspProc(&hProc);
				DSP_ERROR_EXIT(status,"Couldn't attach to DSP",EXIT);

                DEBUG_PRINT("Succesfully recovered from Dsp Crash.\n");
            }
            else if (index == 2) { /* Dsp State Change detect */
                DEBUG_PRINT("Retrieving new state of DSP...\n");
                DSPProcessor_GetState(hProc, &ProcStatus, sizeof(ProcStatus));
                DEBUG_PRINT("New proc state = %d\n", ProcStatus.iState);
                if (ProcStatus.iState == PROC_ERROR) {
                    DEBUG_PRINT("Dsp has changed to Error State, restarting dsp...\n");

                    Dettach_DspProc(hProc);
                    status = Recover_Dsp(&hProc);
                    DSP_ERROR_EXIT(status, "Couldn't recover from DSP Error",EXIT);

	 				status = Attach_DspProc(&hProc);
					DSP_ERROR_EXIT(status,"Couldn't attach to DSP",EXIT);

                    DEBUG_PRINT("Succesfully reloaded dsp.\n");
                }
            }
			else if (index == 3) {
				DEBUG_PRINT("Retrieving current state of DSP...\n");
                DSPProcessor_GetState(hProc, &ProcStatus, sizeof(ProcStatus));
				DEBUG_PRINT("DSP_PWRERROR, DSP state = %d\n",  ProcStatus.iState);
			}
        }
    }

EXIT:
    Dettach_DspProc(hProc);
    DspManager_Close(0,NULL);

    free(notificationObjects[0]);
    free(notificationObjects[1]);
    free(notificationObjects[2]);
	free(notificationObjects[3]);

    LOG_INFO_PRINT("TI DSP Monitor Exiting\n");

    return status;
}

DSP_STATUS Recover_Dsp(DSP_HPROCESSOR *hProcessor)
{
    DSP_STATUS status = DSP_SOK;
    struct DSP_PROCESSORSTATE ProcStatus;

    DSP_HPROCESSOR hProc;

    LOG_ERROR_PRINT("***** TI DSP Monitor Recovering from DSP Crash *****\n");

RETRY:
    status = Attach_DspProc(&hProc);
    DSP_ERROR_EXIT(status,"Couldn't attach to DSP",EXIT);

    status = Load_Baseimage(hProc);

    DSPProcessor_GetState(hProc, &ProcStatus, sizeof(ProcStatus));
    Dettach_DspProc(hProc);

EXIT:
    /* keep trying to reload the baseimage until it can */
    DEBUG_PRINT("Proc state = %d\n", ProcStatus.iState);
    if ((status != DSP_SOK) || (ProcStatus.iState == PROC_ERROR))  {
        sleep(1);
        goto RETRY;
    }

    return status;
}

DSP_STATUS Attach_DspProc(DSP_HPROCESSOR *hProc) {
    DSP_STATUS 	status = DSP_SOK;
    UINT index = 0;
    struct DSP_PROCESSORINFO dspInfo;
    unsigned int numProcs;
    unsigned int uProcId = 0;

    DEBUG_PRINT("Attaching Dsp Processor\n");

#if 0
    status = DspManager_Open(0,NULL);
    DSP_ERROR_EXIT(status,"Couldn't open DSP Manager",EXIT);
    DEBUG_PRINT("Opened DspManager\n");
#endif

    /* Looking for DSP Processors to attach to */
    while (1) {
        status = DSPManager_EnumProcessorInfo(index,&dspInfo,(unsigned int)sizeof(struct DSP_PROCESSORINFO),&numProcs);
        if (DSP_FAILED(status)) break;

        if ((dspInfo.uProcessorType == DSPTYPE_55) ||
            (dspInfo.uProcessorType == DSPTYPE_64)) {
            uProcId = index;
            status = DSP_SOK;
            break;
        }
        index++;
    }
    DSP_ERROR_EXIT(status,"Cannot find DSP Proc",EXIT);

    DEBUG_PRINT("Attaching to Dsp Processor\n");
    status = DSPProcessor_Attach(0, NULL, hProc);

EXIT:
    return status;
}

DSP_STATUS Dettach_DspProc(DSP_HPROCESSOR hProc) {
    DSP_STATUS 	status = DSP_SOK;

    status = DSPProcessor_Detach(hProc);

#if 0
    DspManager_Close(0,NULL);
#endif

    DEBUG_PRINT("Dettached Processor\n");

    return status;
}

DSP_STATUS Load_Baseimage(DSP_HPROCESSOR hProc)
{
    DSP_STATUS status = DSP_SOK;
    const char* argv[2] = {BASEIMAGE_FILE, NULL};

    if (DSP_SUCCEEDED(status)) {
        DEBUG_PRINT("Stopping the DSPProcessor\n");
        status = DSPProcessor_Stop(hProc);
        DSP_ERROR_EXIT(status, "Unable to stop DSPProcessor", EXIT);
        DEBUG_PRINT("Stopped DSPProcessor\n");

        status = DSPProcessor_Load(hProc,1,(const char **)argv,NULL);
        DSP_ERROR_EXIT(status, "Unable to load baseimage", EXIT);
        DEBUG_PRINT("Succesfully loaded baseimage!\n");

        status = DSPProcessor_Start(hProc);
        DSP_ERROR_EXIT(status, "Unable to start DSPProcessor", EXIT);
        DEBUG_PRINT("Starting DSPProcessor\n");
    }
EXIT:
    return status;
}


