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
/*============================================================================
 *  @file   MemMgr.c
 *
 *  @brief  TILER Client Sample application for TILER module between MPU & Ducati
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>
#include <String.h>
#include <Trace.h>

/* IPC headers */
#include <IpcUsr.h>
#include <ProcMgr.h>

/* RCM headers */
#include <RcmServer.h>

/* TILER headers */
#include <tilermgr.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */

#define RCMSERVER_NAME              "MEMALLOCMGRSERVER"

RcmServer_Handle rcmServerHandle;
Int              status;
sem_t            semMemMgrWait;
pthread_t        mmuFaultHandle = 0;


/*
 * Remote function argument structures
 *
 * All fields are simple UInts and Ptrs.  Some arguments may be smaller than
 * these fields, which is okay, as long as they are correctly "unpacked" by the
 * server.
 */

typedef struct {
    UInt    pixelFormat;
    UInt    width;
    UInt    height;
    UInt    length;
    UInt    stride;
    Ptr     ptr;
    UInt  * reserved;
} AllocParams;

typedef struct {
    UInt        numBuffers;
    AllocParams params [1];
} AllocArgs;

typedef struct {
    Ptr bufPtr;
} FreeArgs, ConvertPageModeToTilerSpaceArgs;

typedef struct {
    Ptr     bufPtr;
    UInt    rotationAndMirroring;
} ConvertToTilerSpaceArgs;

typedef struct {
    UInt32 code;
} DebugArgs;


typedef struct  {  /* TODO: remove */
    enum pixel_fmt_t pixelFormat;  /* pixel format */
    union {
        struct {
            pixels_t width;  /* width of 2D buffer */
            pixels_t height; /* height of 2D buffer */
        };
        struct {
            bytes_t  length;  /* length of 1D buffer.  Must be multiple of
                                stride if stride is not 0. */
        };
    };
    unsigned long   stride;    /* must be multiple of page size.  Can be 0 only
                                if pixelFormat is KPAGE. */
    void          * ptr;               /* pointer to beginning of buffer */
    unsigned long   reserved;  /* system space address (used internally) */
} MemMgrBlock;


/*
 *  ======== signalHandler ========
 */
static Void signalHandler (Int sig)
{
    Osal_printf ("\nexiting from the memmgr application\n ");
    sem_post (&semMemMgrWait);
}

/*
 *  ======== mmuFaultHandler ========
 */
static Void mmuFaultHandler (Void)
{
    Int               status;
    UInt              i;
    ProcMgr_EventType events[] = {PROC_STOP, PROC_MMU_FAULT};

    status = ProcMgr_waitForMultipleEvents (PROC_SYSM3, events, 2, -1, &i);
    if (status < 0) {
        Osal_printf ("\nProcMgr_waitForEvent failed status %d\n ", status);
    }
    else {
        Osal_printf ("\nFatal error received!\n ");
    }

    Osal_printf ("\nexiting from the memmgr application\n ");
    sem_post (&semMemMgrWait);
}

/*
 *  ======== getAccessMode ========
 *  helper func to determine bit mode
 */
static
UInt getAccessMode (Ptr bufPtr)
{
    UInt addr = (UInt)bufPtr;

    /*
     * The access mode decoding is as follows:
     *
     * 0x60000000 - 0x67FFFFFF : 8-bit
     * 0x68000000 - 0x6FFFFFFF : 16-bit
     * 0x70000000 - 0x77FFFFFF : 32-bit
     * 0x77000000 - 0x7FFFFFFF : Page mode
     */
    switch (addr & 0xf8000000) {   /* Mask out the lower bits */
    case 0x60000000:
        return PIXEL_FMT_8BIT;
    case 0x68000000:
        return PIXEL_FMT_16BIT;
    case 0x70000000:
        return PIXEL_FMT_32BIT;
    case 0x78000000:
        return PIXEL_FMT_PAGE;
    default:        /* TODO: How to handle invalid case? */
        return 0;
    }
}

/*
 *  ======== getStride ========
 *  helper func to determine stride length
 */
static
UInt getStride (Ptr bufPtr)
{
    switch (getAccessMode (bufPtr)) {
    case PIXEL_FMT_8BIT:
        return 0x4000;  /* 16 KB of stride */
    case PIXEL_FMT_16BIT:
    case PIXEL_FMT_32BIT:
        return 0x8000;  /* 32 KB of stride */
    default:
        return 0;       /* Stride not applicable */
    }
}

/*
 *  ======== fxnMemMgr_Debug ========
 *     RCM function for debugging
*/
static
Int32 fxnMemMgr_Debug (UInt32 dataSize, UInt32 *data)
{
    Osal_printf ("Executing MemMgr_Debug\n");

    /* To be implemented (optional) */

    return 0;
}

/*
 *  ======== fxnMemMgr_Alloc ========
 *     RCM function for MemMgr_Alloc function
*/
static
Int32 fxnMemMgr_Alloc (UInt32 dataSize, UInt32 *data)
{
    AllocArgs     * args = (AllocArgs *)data;
    Int             i;
    MemMgrBlock   * params;
    Ptr             allocedPtr;

    Osal_printf ("Executing MemMgr_Alloc with params:\n");
    Osal_printf ("\tnumBuffers = %d\n", args->numBuffers);
    for(i = 0; i < args->numBuffers; i++) {
        Osal_printf ("\tparams [%d].pixelFormat = %d\n", i,
                        args->params [i].pixelFormat);
        Osal_printf ("\tparams [%d].width = %d\n", i, args->params [i].width);
        Osal_printf ("\tparams [%d].height = %d\n", i, args->params [i].height);
        Osal_printf ("\tparams [%d].length = %d\n", i, args->params [i].length);
    }

    params = (MemMgrBlock *) malloc (sizeof(MemMgrBlock) * args->numBuffers);

    if (params == NULL) {
        Osal_printf ("Error allocating array of MemMgrBlock params.\n");
        return (Int32)NULL;
    }

    for(i = 0; i < args->numBuffers; i++) {
        params [i].pixelFormat = args->params [i].pixelFormat;

        params [i].width = args->params [i].width;
        /* TODO: provide length support on Ducati */
        params [i].height = args->params [i].height;
        params [i].length = args->params [i].length;
    }

    /* Allocation */
    /*
    allocedPtr = MemMgr_Alloc(paramsm, args->numBuffers);
    for(i = 0; i < args->numBuffers; i++) {
        args->params [i].stride = args->params [i].stride;
        args->params [i].ptr = args->params [i].ptr;
    }
    */

    /* Allocation */
    for (i = 0; i < args->numBuffers; i++) {
        switch (params [i].pixelFormat) {
        case PIXEL_FMT_8BIT:
        case PIXEL_FMT_16BIT:
        case PIXEL_FMT_32BIT:
            Osal_printf ("fxnMemMgr_Alloc: calling TilerMgr_Alloc.\n");
            args->params [i].ptr = (Ptr)TilerMgr_Alloc (params [i].pixelFormat,
                                        params [i].width, params [i].height);
            break;
        case PIXEL_FMT_PAGE:
            Osal_printf ("fxnMemMgr_Alloc: calling TilerMgr_PageModeAlloc.\n");
            args->params [i].ptr = \
                                (Ptr)TilerMgr_PageModeAlloc (params [i].length);
            break;
        default:    /* Invalid case */
            Osal_printf ("fxnMemMgr_Alloc: Invalid pixel format.\n");
            args->params [i].ptr = NULL;
            break;
        }
        args->params [i].stride = getStride (args->params [i].ptr);
    }

    allocedPtr = args->params [0].ptr;
    free (params);

    Osal_printf ("fxnMemMgr_Alloc done.\n");
    return (Int32) allocedPtr; /* Return first buffer pointer */
}



/*
 *  ======== fxnMemMgr_Free ========
 *     RCM function for MemMgr_Free
 */
static
Int32 fxnMemMgr_Free (UInt32 dataSize, UInt32 *data)
{
    FreeArgs  * args    = (FreeArgs *)data;
    UInt32      status  = 0;

    Osal_printf ("Executing MemMgr_Free with params:\n");
    Osal_printf ("\tbufPtr = 0x%x\n", args->bufPtr);

    switch (getAccessMode (args->bufPtr)) {
    case PIXEL_FMT_8BIT:
    case PIXEL_FMT_16BIT:
    case PIXEL_FMT_32BIT:
        Osal_printf ("fxnMemAlloc_Free: calling TilerMgr_Free.\n");
        status = TilerMgr_Free ((Int)args->bufPtr);
        break;
    case PIXEL_FMT_PAGE:
        Osal_printf ("fxnMemAlloc_Free: calling TilerMgr_PageModeFree.\n");
        status = TilerMgr_PageModeFree ((Int)args->bufPtr);
        break;
    default:    /* Invalid case */
        Osal_printf ("fxnMemAlloc_Free: Invalid pointer.\n");
        break;
    }

    Osal_printf ("fxnMemMgr_Free done.\n");
    return status;
}

/*
 *  ======== fxnTilerMem_ConvertToTilerSpace ========
 *     RCM function for TilerMem_ConvertToTilerSpace
 */
static
Int32 fxnTilerMem_ConvertToTilerSpace (UInt32 dataSize, UInt32 *data)
{
    ConvertToTilerSpaceArgs   * args = (ConvertToTilerSpaceArgs *)data;
    UInt32                      addr;

    Osal_printf ("Executing TilerMem_ConvertToTilerSpace with params:\n");
    Osal_printf ("\tbufPtr = 0x%x\n", args->bufPtr);
    Osal_printf ("\trotationAndMirroring = 0x%x\n", args->rotationAndMirroring);

    //Stubbed out pending implementation
    /*addr = TilerMem_ConvertToTilerSpace (args->bufPtr,
                                            args->rotationAndMirroring);*/
    addr = TRUE;

    return addr;
}

/*
 *  ======== fxnTilerMem_ConvertPageModeToTilerSpace ========
 *     RCM function for TilerMem_ConvertPageModeToTilerSpace
 */
static
Int32 fxnTilerMem_ConvertPageModeToTilerSpace (UInt32 dataSize, UInt32 *data)
{
    ConvertPageModeToTilerSpaceArgs   * args = \
                                        (ConvertPageModeToTilerSpaceArgs *)data;
    UInt32                              addr;

    Osal_printf ("Executing TilerMem_ConvertPageModeToTilerSpace with params:");
    Osal_printf ("\n\tbufPtr = 0x%x\n", args->bufPtr);

    //Stubbed out pending implementation
    //addr = TilerMem_ConvertPageModeToTilerSpace (args->bufPtr);
    addr = TRUE;

    return addr;
}

struct MemMgr_funcInfo {
    RcmServer_MsgFxn fxnPtr;
    String           name;
};

struct MemMgr_funcInfo memMgrFxns [] =
{
    { fxnMemMgr_Alloc,                        "MemMgr_Alloc"},
    { fxnMemMgr_Free,                         "MemMgr_Free"},
    { fxnMemMgr_Debug,                        "MemMgr_Debug"},
    { fxnTilerMem_ConvertToTilerSpace,        "TilerMem_ConvertToTilerSpace"},
    { fxnTilerMem_ConvertPageModeToTilerSpace,
                                        "TilerMem_ConvertPageModeToTilerSpace"},
};

/*
 *  ======== MemMgrThreadFxn ========
 *     TILER server thread function
*/
Void MemMgrThreadFxn (Char * rcmServerName)
{
    RcmServer_Params    rcmServerParams;
    UInt                fxnIdx;
    Int                 numFxns;
    Int                 i;

    /* RCM Server module init */
    Osal_printf ("RCM Server module init.\n");
    RcmServer_init ();

    /* rcm client module params init*/
    Osal_printf ("RCM Server module params init.\n");
    status = RcmServer_Params_init (&rcmServerParams);
    if (status < 0) {
        Osal_printf ("Error in RCM Server instance params init \n");
        goto exit_rcmserver_destroy;
    } else {
        Osal_printf ("RCM Server instance params init passed \n");
    }

    /* create the RcmServer instance */
    Osal_printf ("Creating RcmServer instance with name %s.\n", rcmServerName);
    status = RcmServer_create (rcmServerName, &rcmServerParams,
                                &rcmServerHandle);
    if (status < 0) {
        Osal_printf ("Error in RCM Server create.\n");
        goto exit_rcmserver_destroy;
    } else {
        Osal_printf ("RCM Server Create passed \n");
    }

    numFxns = sizeof (memMgrFxns) / sizeof (struct MemMgr_funcInfo);
    for (i = 0; i < numFxns; i++) {
        status = RcmServer_addSymbol (rcmServerHandle, memMgrFxns [i].name,
                            memMgrFxns [i].fxnPtr, &fxnIdx);
        /* Register the remote functions */
        Osal_printf ("Registering remote function %s with index %d\n",
                        memMgrFxns [i].name, fxnIdx);
        if (status < 0) {
            Osal_printf ("Add symbol failed with status 0x%08x.\n", status);
        }
    }

    Osal_printf ("Start RCM server thread \n");

    RcmServer_start (rcmServerHandle);
    Osal_printf ("RCM Server start passed \n");

    status = TilerMgr_Open ();
    if (status < 0) {
        Osal_printf ("Error in TilerMgr_Open: status = 0x%x\n", status);
        goto exit_rcmserver_remove_symbol;
    }

    Osal_printf ("\nDone initializing RCM server.  Ready to receive requests "
                    "from Ducati.\n");

    /* wait for commands */
    sem_init (&semMemMgrWait, 0, 0);
    sem_wait (&semMemMgrWait);

    status = TilerMgr_Close ();
    if (status < 0) {
        Osal_printf ("Error in TilerMgr_Close: status = 0x%x\n", status);
    }

    sem_destroy (&semMemMgrWait);

exit_rcmserver_remove_symbol:
    for (i = 0; i < numFxns; i++) {
        /* Unregister the remote functions */
        status = RcmServer_removeSymbol (rcmServerHandle, memMgrFxns [i].name);
        if (status < 0) {
            Osal_printf ("Remove symbol %s failed.\n", memMgrFxns [i].name);
        }
    }

    status = RcmServer_delete (&rcmServerHandle);
    if (status < 0) {
        Osal_printf ("Error in RcmServer_delete: status = 0x%x\n", status);
        goto exit;
    }

exit_rcmserver_destroy:
    RcmServer_exit ();

exit:
    Osal_printf ("Leaving RCM server test thread function \n");
    return;
}

Int main (Int argc, Char * argv [])
{
    Int         status      = 0;
    Ipc_Config  config;

    Ipc_getConfig (&config);
    status = Ipc_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in Ipc_setup [0x%x]\n", status);
        goto exit;
    }

    /* Setup the signal handlers*/
    signal (SIGINT, signalHandler);
    signal (SIGKILL, signalHandler);
    signal (SIGTERM, signalHandler);

    /* Create the MMU fault handler thread */
    Osal_printf ("Create MMU fault handler thread.\n");
    pthread_create (&mmuFaultHandle, NULL,
                        (Void *)&mmuFaultHandler, NULL);

    MemMgrThreadFxn (RCMSERVER_NAME);

    status = Ipc_destroy ();
    if (status < 0) {
        Osal_printf ("Error in Ipc_destroy: status = 0x%x\n", status);
    }

exit:
    Osal_printf ("Exiting memmgr application.\n");
    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
