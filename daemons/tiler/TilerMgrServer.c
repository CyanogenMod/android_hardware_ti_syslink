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
 *  @file   TilerServer.c
 *
 *  @brief  TILER Client Sample application for TILER module between MPU & Ducati
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
#include <../src/tilermgr/tilermgr.h>
#include <TilerMgrServer_config.h>


#define TILERMGRSERVER_DEBUG  0
#define Debug_printf if(TILERMGRSERVER_DEBUG) Osal_printf

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

RcmServer_Handle rcmServerHandle;
Int status;


/*
 * Remote function argument structures
 *
 * All fields are simple UInts and Ptrs.  Some arguments may be smaller than
 * these fields, which is okay, as long as they are correctly "unpacked" by the
 * server.
 */
typedef struct {
    UInt pixelFormat;
    UInt width;
    UInt height;
} TilerAllocArgs;

typedef struct {
    Ptr oldBuffer;
    UInt newWidth;
    UInt newHeight;
} TilerReallocArgs;

typedef struct {
    Ptr tilerBuffer;
} TilerFreeArgs;

typedef struct {
    UInt length;
} TilerPageModeAllocArgs;

typedef struct {
    Ptr oldBuffer;
    UInt newLength;
} TilerPageModeReallocArgs;

typedef struct {
    Ptr tilerBuffer;
} TilerPageModeFreeArgs;

typedef struct {
    Ptr systemPointer;
    UInt rotationAndMirroring;
} ConvertToTilerSpaceArgs;

typedef struct {
    Ptr systemPointer;
} ConvertPageModeToTilerSpaceArgs;

typedef struct {
    Ptr virtualPointer;
} ConvertToSystemSpaceArgs;

typedef struct {
    UInt32 code;
} DebugArgs;

/*
 *    Union of all args so we know the maximum message size
 */
typedef union {
    TilerAllocArgs                  tilerAllocArgs;
    TilerReallocArgs                tilerReallocArgs;
    TilerFreeArgs                   tilerFreeArgs;
    TilerPageModeAllocArgs          tilerPageModeAllocArgs;
    TilerPageModeReallocArgs        tilerPageModeReallocArgs;
    TilerPageModeFreeArgs           tilerPageModeFreeArgs;
    ConvertToTilerSpaceArgs         convertToTilerSpaceArgs;
    ConvertPageModeToTilerSpaceArgs convertPageModeToTilerSpaceArgs;
    ConvertToSystemSpaceArgs        convertToSystemSpaceArgs;
    DebugArgs                       debugArgs;
} TilerFxnArgs;


/*
 *  ======== convertToVirtualSpace ========
 *     Convert system space address to virtual space address
*/
Ptr convertToVirtualSpace(Ptr ssPtr)
{
    // For now, simply return the original pointer
    // as system space and virtual space are the same

    return ssPtr;
}

/*
 *  ======== fxnTilerMgrServerDebug ========
 *     RCM function for debugging
*/
static
Int32 fxnTilerMgrServerDebug(UInt32 dataSize, UInt32 *data)
{
    Debug_printf("Executing fxnTilerMgrServerDebug\n");

    // To be implemented (optional)

    return 0;
}

#define TILERMGRSERVER_DEBUG 1
/*
 *  ======== fxnTilerAlloc ========
 *     RCM function for tilerAlloc function
*/
static
Int32 fxnTilerAlloc(UInt32 dataSize, UInt32 *data)
{
    TilerAllocArgs *args = (TilerAllocArgs *)data;
    Ptr ssPtr, vsPtr;

    Debug_printf("Executing fxnTilerAlloc with params:\n");
    Debug_printf("\tpixelFormat = %d\n", args->pixelFormat);
    Debug_printf("\twidth = %d\n", args->width);
    Debug_printf("\theight = %d\n", args->height);

    ssPtr = tilerAlloc(args->pixelFormat, args->width, args->height, 0);
    vsPtr = convertToVirtualSpace(ssPtr);

    Debug_printf("fxnTilerAlloc returning pointer 0x%x\n", vsPtr);

    return (Int32)vsPtr;
}
#define TILERMGRSERVER_DEBUG 0

/*
 *  ======== fxnTilerRelloc ========
 *     RCM function for tilerRealloc function
*/
static
Int32 fxnTilerRealloc(UInt32 dataSize, UInt32 *data)
{
    TilerReallocArgs *args = (TilerReallocArgs *)data;
    Ptr ssPtr, vsPtr;

    Debug_printf("Executing fxnTilerRealloc with params:\n");
    Debug_printf("\toldBuffer = %d\n", args->oldBuffer);
    Debug_printf("\tnewWidth = %d\n", args->newWidth);
    Debug_printf("\tnewHeight = %d\n", args->newHeight);

    ssPtr = tilerRealloc(args->oldBuffer, args->newWidth, args->newHeight);
    vsPtr = convertToVirtualSpace(ssPtr);

    Debug_printf("fxnTilerRealloc returning pointer 0x%x\n", vsPtr);

    return (Int32)vsPtr;
}
#define TILERMGRSERVER_DEBUG 1

/*
 *  ======== fxnTilerFree ========
 *     RCM function for tilerFree function
*/
static
Int32 fxnTilerFree(UInt32 dataSize, UInt32 *data)
{
    TilerFreeArgs *args = (TilerFreeArgs *)data;

    Debug_printf("Executing fxnTilerFree with params:\n");
    Debug_printf("\ttilerBuffer = 0x%x\n", args->tilerBuffer);

    tilerFree(args->tilerBuffer);

    return 0;
}
#define TILERMGRSERVER_DEBUG 0

/*
 *  ======== fxnTilerPageModeAlloc ========
 *     RCM function for tilerPageModeAlloc function
*/
static
Int32 fxnTilerPageModeAlloc(UInt32 dataSize, UInt32 *data)
{
    TilerPageModeAllocArgs *args = (TilerPageModeAllocArgs *)data;
    Ptr ssPtr, vsPtr;

    Debug_printf("Executing fxnTilerPageModeAlloc with params:\n");
    Debug_printf("\tlength = %d\n", args->length);

    ssPtr = tilerPageModeAlloc(args->length);
    vsPtr = convertToVirtualSpace(ssPtr);

    Debug_printf("fxnTilerPageModeAlloc returning pointer 0x%x\n", vsPtr);

    return (Int32)vsPtr;
}


/*
 *  ======== fxnTilerPageModeRealloc ========
 *     RCM function for tilerPageModeRealloc function
*/
static
Int32 fxnTilerPageModeRealloc(UInt32 dataSize, UInt32 *data)
{
    TilerPageModeReallocArgs *args = (TilerPageModeReallocArgs *)data;
    Ptr ssPtr, vsPtr;

    Debug_printf("Executing fxnTilerPageModeRealloc with params:\n");
    Debug_printf("\toldBuffer = 0x%x\n", args->oldBuffer);
    Debug_printf("\tnewLength = %d\n", args->newLength);

    ssPtr = tilerPageModeRealloc(args->oldBuffer, args->newLength);
    vsPtr = convertToVirtualSpace(ssPtr);

    Debug_printf("fxnTilerPageModeRealloc returning pointer 0x%x\n", vsPtr);
    
    return (Int32)vsPtr;
}
/*
 *  ======== fxnTilerPageModeFree ========
 *     RCM function for tilerPageModeFree function
*/
static
Int32 fxnTilerPageModeFree(UInt32 dataSize, UInt32 *data)
{
    TilerPageModeFreeArgs *args = (TilerPageModeFreeArgs *)data;

    Debug_printf("Executing fxnTilerPageModeFree with params:\n");
    Debug_printf("\ttilerBuffer = 0x%x\n", args->tilerBuffer);

    tilerPageModeFree(args->tilerBuffer);

    return 0;
}

/*
 *  ======== fxnConvertToTilerSpace ========
 *     RCM function for convertToTilerSpace function
*/
static
Int32 fxnConvertToTilerSpace(UInt32 dataSize, UInt32 *data)
{
    ConvertToTilerSpaceArgs *args = (ConvertToTilerSpaceArgs *)data;
    Ptr tsPtr;

    Debug_printf("Executing fxnConvertToTilerSpace with params:\n");
    Debug_printf("\tsystemPointer = 0x%x\n", args->systemPointer);
    Debug_printf("\trotationAndMirroring = %d\n", args->rotationAndMirroring);

    tsPtr = convertToTilerSpace(args->systemPointer, args->rotationAndMirroring);

    Debug_printf("fxnConvertToTilerSpace returning pointer 0x%x\n", tsPtr);

    return (Int32)tsPtr;
}

/*
 *  ======== fxnConvertPageModeToTilerSpace ========
 *     RCM function for convertPageModeToTilerSpace function
*/
static
Int32 fxnConvertPageModeToTilerSpace(UInt32 dataSize, UInt32 *data)
{
    ConvertPageModeToTilerSpaceArgs *args = (ConvertPageModeToTilerSpaceArgs *)data;
    Ptr tsPtr;

    Debug_printf("Executing fxnConvertPageModeToTilerSpace with params:\n");
    Debug_printf("\tsystemPointer = 0x%x\n", args->systemPointer);

    tsPtr = convertPageModeToTilerSpace(args->systemPointer);

    Debug_printf("fxnConvertPageModeToTilerSpace returning pointer 0x%x\n", tsPtr);
    
    return (Int32)tsPtr;
}

/*
 *  ======== fxnConvertToSystemSpace ========
 *     RCM function for converting virtual space pointer to
 *  system space pointer
*/
static
Int32 fxnConvertToSystemSpace(UInt32 dataSize, UInt32 *data)
{
    ConvertToSystemSpaceArgs *args = (ConvertToSystemSpaceArgs *)data;
    Ptr ssPtr;

    Debug_printf("Executing fxnConvertToSystemSpace with params:\n");
    Debug_printf("\tvirtualPointer = 0x%x\n", args->virtualPointer);

    // For now, simply return the original pointer
    ssPtr = args->virtualPointer;

    Debug_printf("fxnConvertToSystemSpace returning pointer 0x%x\n", ssPtr);

    return (Int32)ssPtr;
}

struct tilerserver_func_info {
    RcmServer_RemoteFuncPtr func_ptr;
    String name;
};



struct tilerserver_func_info tilerFxns[] = 
{
    { fxnTilerAlloc, "fxnTilerAlloc"},
    { fxnTilerRealloc, "fxnTilerRealloc"},
    { fxnTilerFree, "fxnTilerFree"},
    { fxnTilerPageModeAlloc, "fxnTilerPageModeAlloc"},
    { fxnTilerPageModeRealloc, "fxnTilerPageModeRealloc"},
    { fxnTilerPageModeFree, "fxnTilerPageModeFree"},
    { fxnConvertToTilerSpace, "fxnConvertToTilerSpace"},
    { fxnConvertPageModeToTilerSpace, "fxnConvertPageModeToTilerSpace"},
    { fxnConvertToSystemSpace, "fxnConvertToSystemSpace"},
    { fxnTilerMgrServerDebug, "fxnTilerMgrServerDebug"},
};

/*
 *  ======== TilerServerThreadFxn ========
 *     TILER server thread function
*/
Void TilerServerThreadFxn()
{
    RcmServer_Config                cfgParams;
    RcmServer_Params                rcmServer_Params;
    Char *                          rcmServerName = RCMSERVER_NAME;
    UInt                            fxnIdx;
    Char *                          procNameAppM3;
    Char *                          procNameSysM3;
    UInt16                          localId = MULTIPROC_INVALIDID;
    UInt16                          procId;
    UInt16                          remoteIdSysM3;
    UInt16                          remoteIdAppM3;
    Handle                          procMgrHandle = NULL;
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
    UInt32                          curShAddrSysM3;
    UInt32                          curShAddrAppM3;
    UInt32                          nsrnEventNoSysM3;
    UInt32                          mqtEventNoSysM3;
    UInt32                          nsrnEventNoAppM3;
    UInt32                          mqtEventNoAppM3;
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config                   config;
    UInt32 entry_point = 0;
    ProcMgr_StartParams start_params;
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
    MultiProc_Config                 multiProcConfig;
    NotifyDriverShm_Handle          notifyDrvHandle;
    NotifyDriverShm_Params          notifyShmParams;
    GatePeterson_Handle             gateHandle = NULL;
    GatePeterson_Handle             gateHandleSysM3 = NULL;
    GatePeterson_Handle             gateHandleAppM3 = NULL;
    GatePeterson_Params             gateParams;
    NameServerRemoteNotify_Handle   nsrnHandle;
    NameServerRemoteNotify_Params   nsrParams;
    MessageQTransportShm_Handle     transportShmHandle;
    MessageQTransportShm_Params     msgqTransportParams;
#endif /* if defined(SYSLINK_USE_SYSMGR) */
    ProcMgr_Handle                  procMgrHandle_server;
    Int                             num_of_funcs;
    Int                             i;
    Bool                            sysM3Client = FALSE;
    Bool                            appM3Client = FALSE;

    /* Must handle both AppM3 and SysM3 as remote processor */
    procNameSysM3       = SYSM3_PROC_NAME;
    nsrnEventNoSysM3    = NSRN_NOTIFY_EVENTNO;
    mqtEventNoSysM3     = TRANSPORT_NOTIFY_EVENTNO;

    procNameAppM3       = APPM3_PROC_NAME;
    nsrnEventNoAppM3    = NSRN_NOTIFY_EVENTNO1;
    mqtEventNoAppM3     = TRANSPORT_NOTIFY_EVENTNO1;

    #if defined (TILER_SYSM3)
        sysM3Client = TRUE;
        appM3Client = FALSE;
    #else
        sysM3Client = FALSE;    /* Default case = AppM3 */
        appM3Client = TRUE;
    #endif    

    /* Setup MultiProc. */
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
    }
#endif

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }
#else /* if defined(SYSLINK_USE_SYSMGR) */
    if (status >= 0) {
        UsrUtilsDrv_setup ();

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
            Debug_printf ("Error in SharedRegion_setup. Status [0x%x]\n",
                         status);
        }
        else {
            Debug_printf ("SharedRegion_setup Status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* ListMPSharedMemory module setup */
        ListMPSharedMemory_getConfig (&listMPSharedConfig);
        status = ListMPSharedMemory_setup (&listMPSharedConfig);
        if (status < 0) {
            Debug_printf ("Error in ListMPSharedMemory_setup."
                         " Status [0x%x]\n",
                         status);
        }
        else  {
            Debug_printf ("ListMPSharedMemory_setup Status [0x%x]\n",
                         status);

            /* HeapBuf module setup */
            HeapBuf_getConfig (&heapbufConfig);
            status = HeapBuf_setup (&heapbufConfig);
            if (status < 0) {
                Debug_printf ("Error in HeapBuf_setup. Status [0x%x]\n",
                status);
            }
            else {
                Debug_printf ("HeapBuf_setup Status [0x%x]\n", status);
            }
        }
    }

    if (status >= 0) {
        /* GatePeterson module setup */
        GatePeterson_getConfig (&gpConfig);
        status = GatePeterson_setup (&gpConfig);
        if (status < 0) {
            Debug_printf ("Error in GatePeterson_setup. Status [0x%x]\n",
                         status);
        }
        else {
            Debug_printf ("GatePeterson_setup Status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* Setup Notify module and NotifyDriverShm module */
        Notify_getConfig (&notifyConfig);
        notifyConfig.maxDrivers = 2;
        status = Notify_setup (&notifyConfig);
        if (status < 0) {
            Debug_printf ("Error in Notify_setup [0x%x]\n", status);
        }
        else {
            Debug_printf ("Notify_setup Status [0x%x]\n", status);
            NotifyDriverShm_getConfig (&notifyDrvShmConfig);
            status = NotifyDriverShm_setup (&notifyDrvShmConfig);
            if (status < 0) {
                Debug_printf ("Error in NotifyDriverShm_setup [0x%x]\n",
                             status);
            }
            else {
                Debug_printf ("NotifyDriverShm_setup Status [0x%x]\n", status);
            }
        }
    }

    if (status >= 0) {
        /* Setup MessageQ module and MessageQTransportShm module */
        MessageQ_getConfig (&messageqConfig);
        status = MessageQ_setup (&messageqConfig);
        if (status < 0) {
            Debug_printf ("Error in MessageQ_setup [0x%x]\n", status);
        }
        else {
            Debug_printf ("MessageQ_setup Status [0x%x]\n", status);
            MessageQTransportShm_getConfig (&msgqTransportConfig);
            status = MessageQTransportShm_setup (&msgqTransportConfig);
            if (status < 0) {
                Debug_printf ("Error in MessageQTransportShm_setup [0x%x]\n",
                             status);
            }
            else {
                Debug_printf ("MessageQTransportShm_setup Status [0x%x]\n",
                             status);
            }
        }
    }
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    /* Get MultiProc IDs by name. */
    remoteIdSysM3 = MultiProc_getId (SYSM3_PROC_NAME);
    Debug_printf ("MultiProc_getId remoteId: [0x%x]\n", remoteIdSysM3);
    remoteIdAppM3 = MultiProc_getId (APPM3_PROC_NAME);
    Debug_printf ("MultiProc_getId remoteId: [0x%x]\n", remoteIdAppM3);
    procId = MultiProc_getId (SYSM3_PROC_NAME);
    Debug_printf ("MultiProc_getId procId: [0x%x]\n", procId);

    printf("RCM procId= %d\n", procId);
    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle_server,
                           procId);
    if (status < 0) {
        Debug_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        Debug_printf ("ProcMgr_open Status [0x%x]\n", status);
        /* Get the address of the shared region in kernel space. */
        status = ProcMgr_translateAddr (procMgrHandle_server,
                                        (Ptr) &shAddrBase,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);
        if (status < 0) {
            Debug_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                         status);
        }
        else {
            Debug_printf ("Virt address of shared address base #1:"
                         " [0x%x]\n", shAddrBase);
        }

        if (status >= 0) {
            /* Get the address of the shared region in kernel space. */
            status = ProcMgr_translateAddr (procMgrHandle_server,
                                            (Ptr) &shAddrBase1,
                                            ProcMgr_AddrType_MasterUsrVirt,
                                            (Ptr) SHAREDMEM1,
                                            ProcMgr_AddrType_SlaveVirt);
            if (status < 0) {
                Debug_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                             status);
            }
            else {
                Debug_printf ("Virt address of shared address base #2:"
                             " [0x%x]\n", shAddrBase1);
            }
        }
    }
    if (status >= 0) {
        curShAddrSysM3 = shAddrBase;
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (0,
                                   (Ptr) shAddrBase,
                                   SHAREDMEMSIZE);
        if (status < 0) {
            Debug_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (1,
                                   (Ptr) shAddrBase1,
                                   SHAREDMEMSIZE1);
        if (status < 0) {
            Debug_printf ("Error in SharedRegion_add1 [0x%x]\n", status);
        }
    }

#if !defined(SYSLINK_USE_SYSMGR)
    if (status >= 0) {
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

        /* Increment the offset for the next allocation for MPU-SysM3 */
//        curShAddrSysM3 += NOTIFYMEMSIZE;
        curShAddrSysM3 += NOTIFYMEMSIZE + 0x40000;
        /* Reset the address for the next allocation for MPU-AppM3 */
//        curShAddrAppM3 = shAddrBase1;
        curShAddrAppM3 = shAddrBase1 + 0x40000;

        /* Create instance of NotifyDriverShm */
        notifyDrvHandle = NotifyDriverShm_create (
                                                     "NOTIFYDRIVER_DUCATI",
                                                     &notifyShmParams);
        Debug_printf ("NotifyDriverShm_create Handle: [0x%x]\n",
                     notifyDrvHandle);
        if (notifyDrvHandle == NULL) {
            Debug_printf ("Error in NotifyDriverShm_create\n");
        }
    }
#endif


#if defined(SYSLINK_USE_SYSMGR)
    start_params.proc_id = remoteIdSysM3;
    Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
    status  = ProcMgr_start(procMgrHandle_server, entry_point, &start_params);
    Osal_printf ("ProcMgr_start Status [0x%x]\n", status);
    
    if(appM3Client) {
        start_params.proc_id = remoteIdAppM3;
        Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
        status  = ProcMgr_start(procMgrHandle_server, entry_point, &start_params);
        Osal_printf ("ProcMgr_start Status [0x%x]\n", status);
    }
#else
    if (status >= 0) {
        if(sysM3Client) {
            GatePeterson_Params_init (NULL, &gateParams);
            gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
            gateParams.sharedAddr     = (Ptr)(curShAddrSysM3);

            Debug_printf ("Memory required for GatePeterson instance [0x%x]"
                     " bytes \n",
                     gateParams.sharedAddrSize);

            gateHandle = GatePeterson_create (&gateParams);
            if (gateHandle == NULL) {
                Debug_printf ("Error in GatePeterson_create [0x%x]\n", status);
            }
            else {
                Debug_printf ("GatePeterson_create Status [0x%x]\n", status);
            }
            gateHandleSysM3 = gateHandle;
        }
    }

    if (status >= 0) {
        if(appM3Client) {
            GatePeterson_Params_init (NULL, &gateParams);
            gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
            gateParams.sharedAddr     = (Ptr)(curShAddrAppM3);

            Debug_printf ("Memory required for GatePeterson instance [0x%x]"
                     " bytes \n",
                     gateParams.sharedAddrSize);

            gateHandle = GatePeterson_create (&gateParams);
            if (gateHandle == NULL) {
                Debug_printf ("Error in GatePeterson_create [0x%x]\n", status);
            }
            else {
                Debug_printf ("GatePeterson_create Status [0x%x]\n", status);
            }
            gateHandleAppM3 = gateHandle;
        }
    }

    if (status >= 0) {
        if(sysM3Client) {
            /* Increment the offset for the next allocation */
            curShAddrSysM3 += HEAPBUFMEMSIZE + GATEPETERSONMEMSIZE;

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
            nsrParams.notifyEventNo = nsrnEventNoSysM3;
            nsrParams.sharedAddr    = (Ptr)curShAddrSysM3;
            nsrParams.gate          = (Ptr)gateHandleSysM3;
            nsrParams.sharedAddrSize  = NSRN_MEMSIZE;
            nsrnHandle = NameServerRemoteNotify_create(remoteIdSysM3, &nsrParams);
            if (nsrnHandle == NULL) {
                Debug_printf ("Error in NotifyDriverShm_create\n");
            }
            else {
                Debug_printf ("NameServerRemoteNotify_create handle [0x%x]\n",
                             nsrnHandle);
            }
        }
    }

    if (status >= 0) {
        if(appM3Client) {
            /* Increment the offset for the next allocation */
            curShAddrAppM3 += HEAPBUFMEMSIZE + GATEPETERSONMEMSIZE;

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
            nsrParams.notifyEventNo = nsrnEventNoAppM3;
            nsrParams.sharedAddr    = (Ptr)curShAddrAppM3;
            nsrParams.gate          = (Ptr)gateHandleAppM3;
            nsrParams.sharedAddrSize  = NSRN_MEMSIZE;
            nsrnHandle = NameServerRemoteNotify_create(remoteIdAppM3, &nsrParams);
            if (nsrnHandle == NULL) {
                Debug_printf ("Error in NotifyDriverShm_create\n");
            }
            else {
                Debug_printf ("NameServerRemoteNotify_create handle [0x%x]\n",
                             nsrnHandle);
            }
        }
    }

    if (status >= 0) {
        if(sysM3Client) {
            /* Increment the offset for the next allocation */
            curShAddrSysM3 += NSRN_MEMSIZE;

            MessageQTransportShm_Params_init (NULL, &msgqTransportParams);

            msgqTransportParams.sharedAddr = (Ptr)curShAddrSysM3;
            msgqTransportParams.gate = (Gate_Handle) gateHandleSysM3;
            msgqTransportParams.notifyEventNo = mqtEventNoSysM3;
            msgqTransportParams.notifyDriver = notifyDrvHandle;
            msgqTransportParams.sharedAddrSize =
                     MessageQTransportShm_sharedMemReq (&msgqTransportParams);

            transportShmHandle = MessageQTransportShm_create (remoteIdSysM3, &msgqTransportParams);
            if (transportShmHandle == NULL) {
                Debug_printf ("Error in MessageQTransportShm_create\n");
            }
            else {
                Debug_printf ("MessageQTransportShm_create handle [0x%x]\n",
                             transportShmHandle);
            }
        }
    }

    if (status >= 0) {
        if(appM3Client) {
            /* Increment the offset for the next allocation */
            curShAddrAppM3 += NSRN_MEMSIZE;

            MessageQTransportShm_Params_init (NULL, &msgqTransportParams);

            msgqTransportParams.sharedAddr = (Ptr)curShAddrAppM3;
            msgqTransportParams.gate = (Gate_Handle) gateHandleAppM3;
            msgqTransportParams.notifyEventNo = mqtEventNoAppM3;
            msgqTransportParams.notifyDriver = notifyDrvHandle;
            msgqTransportParams.sharedAddrSize =
                     MessageQTransportShm_sharedMemReq (&msgqTransportParams);

            transportShmHandle = MessageQTransportShm_create (remoteIdAppM3, &msgqTransportParams);
            if (transportShmHandle == NULL) {
                Debug_printf ("Error in MessageQTransportShm_create\n");
            }
            else {
                Debug_printf ("MessageQTransportShm_create handle [0x%x]\n",
                             transportShmHandle);
            }
        }
    }
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    /* Get default config for rcm client module */
    Debug_printf ("Get default config for RCM server module.\n");
    status = RcmServer_getConfig (&cfgParams);
    if (status < 0) {
        Debug_printf ("Error in RCM Server module get config \n");
        goto exit;
    } else {
        Debug_printf ("RCM Client module get config passed \n");
    }

    /* rcm client module setup*/
    Debug_printf ("RCM Server module setup.\n");
    status = RcmServer_setup (&cfgParams);
    if (status < 0) {
        Debug_printf ("Error in RCM Server module setup \n");
        goto exit;
    } else {
        Debug_printf ("RCM Server module setup passed \n");
    }

    /* rcm client module params init*/
    Debug_printf ("rcm client module params init.\n");
    status = RcmServer_Params_init (NULL, &rcmServer_Params);
    if (status < 0) {
        Debug_printf ("Error in RCM Server instance params init \n");
        goto exit;
    } else {
        Debug_printf ("RCM Server instance params init passed \n");
    }

    /* create the RcmServer instance */
    Debug_printf ("Creating RcmServer instance.\n");
    status = RcmServer_create (rcmServerName, &rcmServer_Params, &rcmServerHandle);
    if (status < 0) {
        Debug_printf ("Error in RCM Server create.\n");
        goto exit;
    } else {
        Debug_printf ("RCM Server Create passed \n");
    }

    num_of_funcs = sizeof(tilerFxns)/sizeof(struct tilerserver_func_info);
    for (i = 0; i < num_of_funcs; i++) {
        status = RcmServer_addSymbol (rcmServerHandle, tilerFxns[i].name,
                            tilerFxns[i].func_ptr, &fxnIdx);
        /* Register the remote functions */
        Debug_printf ("Registering remote function %s with index %d\n", tilerFxns[i].name, fxnIdx);
        if (status < 0) 
            Debug_printf ("Add symbol failed with status 0x%08x.\n", status);
    }

    Osal_printf ("Start RCM server thread \n");
    status = RcmServer_start(rcmServerHandle);
    if (status < 0) {
        Osal_printf ("Error in RCM Server start.\n");
        goto exit;
    } else {
        Osal_printf ("RCM Server start passed \n");
    }

    Debug_printf ("\nDone initializing RCM server.  Ready to receive requests from Ducati.\n");

    // Loop indefinitely
    while(1) ;

    for (i = 0; i < num_of_funcs; i++) {
        /* Unregister the remote functions */
        Debug_printf ("Unregistering remote function - %d\n", i);
        status = RcmServer_removeSymbol (rcmServerHandle, tilerFxns[i].name);
     if (status < 0)
        Debug_printf ("Remove symbol failed.\n");
    }

exit:
    Debug_printf ("Leaving RCM server test thread function \n");
    return;
}

/*
 *  ======== RcmServerCleanup ========
 */
Void TilerServerCleanup (Void)
{
    /* delete the rcm client */
    Debug_printf ("Delete RCM server instance \n");
    status = RcmServer_delete (&rcmServerHandle);
    if (status < 0) {
        Debug_printf ("Error in RCM Server instance delete\n");
    }

    /* rcm client module destroy*/
    Debug_printf ("Destroy RCM server module \n");
    status = RcmServer_destroy ();
    if (status < 0) {
        Debug_printf ("Error in RCM Server module destroy \n");
    }
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
