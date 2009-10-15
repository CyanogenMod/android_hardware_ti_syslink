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
 *  @file   HeapBufApp.c
 *
 *  @brief  Sample application for HeapBuf module
 *  ============================================================================
 */

/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>

 /* Linux OS-specific headers */
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <OsalPrint.h>
#include <OsalSemaphore.h>
#include <Memory.h>
#include <String.h>

/* Module level headers */
#include <ProcMgr.h>
#include <_ProcMgrDefs.h>

#if defined (SYSLINK_USE_SYSMGR)
#include <SysMgr.h>
#else /* if defined (SYSLINK_USE_SYSMGR) */
#include <UsrUtilsDrv.h>
#include <MultiProc.h>
#include <NameServer.h>
#include <SharedRegion.h>
#include <GatePeterson.h>
#include <ListMPSharedMemory.h>
#include <Heap.h>
#include <HeapBuf.h>
#endif /* if defined(SYSLINK_USE_SYSMGR) */

#include <HeapBufApp.h>
#include "HeapBufApp_config.h"

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
#define PROCMGR_DRIVER_NAME         "/dev/syslink-procmgr"

/*!
 *  @brief  Maximum name length
 */
#define MAX_NAME_LENGTH 32u
/*!
 *  @brief  Maximum name length
 */
#define MAX_VALUE_LENGTH 32u
/*!
 *  @brief  Maximum name length
 */
#define MAX_RUNTIMEENTRIES 10u

#define GATEPETERSONMEMOFFSET    (GATEPETERSONMEM - SHAREDMEM)
#define HEAPBUFMEMOFFSET_1       (HEAPMBMEM_CTRL - SHAREDMEM)
#define HEAPBUFMEMOFFSET_2       (HEAPMBMEM1_CTRL - SHAREDMEM)

ProcMgr_Handle       HeapBufApp_Prochandle   = NULL;
UInt32               curAddr   = 0;
GatePeterson_Handle  gateHandle;
HeapBuf_Handle       heapHandle;
HeapBuf_Handle       heapHandle1;

typedef struct Heap_Block_Tag
{
    char   name[MAX_NAME_LENGTH];
    Int32  id;
}Heap_Block;

/*!
 *  @brief  Function to execute the startup for HeapBufApp sample application
 *
 *  @sa
 */
Int
HeapBufApp_startup (Void)
{
    Int32                     status = 0 ;
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config config;
#else
    GatePeterson_Config       cfgGateParams;
    HeapBuf_Config            cfgHeapBufParams;
    SharedRegion_Config       cfgShrParams;
    ListMPSharedMemory_Config cfgLstParams;
    MultiProc_Config           multiProcConfig;
#endif /* if defined(SYSLINK_USE_SYSMGR) */
    GatePeterson_Params       gateParams;
    NameServer_Params         NameServerParams;
    String                    Gate_Name = "TESTGATE";
    UInt16                    Remote_procId;
    UInt16                    myProcId;

    Osal_printf ("Entered HeapBuf startup\n");

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }
#else /* if defined(SYSLINK_USE_SYSMGR) */
    UsrUtilsDrv_setup ();

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

    /* This will set up the NameServer. NameServer will be used internally by
     *  many modules internally (optinally we can disable it usage in modules)
     */
    NameServerParams.maxNameLen        = MAX_NAME_LENGTH;
    NameServerParams.maxRuntimeEntries = MAX_VALUE_LENGTH;
    NameServerParams.maxValueLen       = MAX_RUNTIMEENTRIES;
    status = NameServer_setup();
    if (status < 0) {
        Osal_printf ("Error in NameServer_setup [0x%x]\n", status);
    }
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    myProcId = MultiProc_getId("MPU");
    Osal_printf ("MultiProc_getId(MPU) = 0x%x]\n", myProcId);

    /* Get MultiProc ID by name for the remote processor. */
    Remote_procId = MultiProc_getId ("SysM3");
    Osal_printf ("MultiProc_getId(SysM3) = 0x%x]\n", Remote_procId);

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&HeapBufApp_Prochandle, Remote_procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        Osal_printf ("ProcMgr_open Status [0x%x]\n", status);
        /* Get the address of the shared region in kernel space. */
        status = ProcMgr_translateAddr (HeapBufApp_Prochandle,
                                        (Ptr) &curAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                         status);
        } else {
            Osal_printf ("Virt address of shared address base:"
                         " [0x%x]\n",
                         curAddr);
        }
    }

#if !defined (SYSLINK_USE_SYSMGR)
    /* Configure shared region (prefered custom setup rather than default) */
    cfgShrParams.gateHandle = NULL;
    cfgShrParams.heapHandle = NULL;
    cfgShrParams.maxRegions = 4;
    status = SharedRegion_setup(&cfgShrParams);
    if (status < 0) {
        Osal_printf ("Error in SharedRegion_setup [0x%x]\n", status);
    }
    else {
#endif /* if !defined(SYSLINK_USE_SYSMGR) */
        SharedRegion_add(0,(Ptr)curAddr, SHAREDMEMSIZE);

#if !defined (SYSLINK_USE_SYSMGR)
        GatePeterson_getConfig (&cfgGateParams);
        cfgGateParams.maxNameLen = MAX_NAME_LENGTH;
        status = GatePeterson_setup (&cfgGateParams);
        if (status < 0) {
            Osal_printf ("Error in GatePeterson_setup [0x%x]\n", status);
        }
        else {
#endif /* if !defined(SYSLINK_USE_SYSMGR) */
            GatePeterson_Params_init(gateHandle, &gateParams);
            gateParams.sharedAddr     = (Ptr)(curAddr + GATEPETERSONMEMOFFSET);
            gateParams.sharedAddrSize = GatePeterson_sharedMemReq(&gateParams);
            gateParams.name           = Memory_alloc(NULL,
                                                     sizeof(Gate_Name),
                                                     0u);
            String_ncpy(gateParams.name, Gate_Name,String_len (Gate_Name));
            Osal_printf ("Run Ducati Sample(If not not done yet)"
                        "  and press any key to continue ...\n");
            getchar ();

            do {
                gateParams.sharedAddr     = (Ptr)(curAddr + GATEPETERSONMEMOFFSET);
                status = GatePeterson_open (&gateHandle, &gateParams);
            }
            while (status < 0);

            if (status < 0) {
                Osal_printf ("Error in GatePeterson_open [0x%x]\n", status);
            }
            else {
                Osal_printf ("GatePeterson_open Status [0x%x]\n", status);
#if !defined (SYSLINK_USE_SYSMGR)
                cfgLstParams.maxNameLen = MAX_NAME_LENGTH;
                status = ListMPSharedMemory_setup(&cfgLstParams);
                if (gateHandle == NULL) {
                    Osal_printf ("Error in GatePeterson_setup [0x%x]\n", status);
                }
                else {
                    HeapBuf_getConfig (&cfgHeapBufParams);
                    cfgHeapBufParams.maxNameLen     = MAX_NAME_LENGTH;
                    cfgHeapBufParams.trackMaxAllocs = TRUE;
                    status = HeapBuf_setup(&cfgHeapBufParams);
                    if (status < 0) {
                        Osal_printf ("Error in HeapBuf_setup [0x%x]\n", status);
                    }
                }
#endif /* if !defined(SYSLINK_USE_SYSMGR) */
            }
#if !defined (SYSLINK_USE_SYSMGR)
        }
    }
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    return 0;
}


/*!
 *  @brief  Function to execute the HeapBufApp sample application
 *
 *  @sa     HeapBufApp_callback
 */
Int
HeapBufApp_execute (Void)
{
    int size = 0;
    Int32          status     = -1;
    HeapBuf_Params heapbufParams;
    HeapBuf_Params heapbufParams1;
    Heap_Block *   heapBlock;
    HeapBuf_ExtendedStats stats;

    HeapBuf_Params_init(NULL,&heapbufParams);

    heapbufParams.name              = NULL;
    heapbufParams.sharedAddr        = (Ptr)(curAddr + HEAPBUFMEMOFFSET_1);
    heapbufParams.align             = 128;
    heapbufParams.numBlocks         = 4;
    heapbufParams.blockSize         = MSGSIZE;
    heapbufParams.gate                 = (Gate_Handle)gateHandle;
    heapbufParams.sharedAddrSize    = HeapBuf_sharedMemReq(&heapbufParams,
                                        &heapbufParams.sharedBufSize);
    heapbufParams.sharedBuf         = (Ptr)(curAddr + HEAPBUFMEMOFFSET_1 +
                                                heapbufParams.sharedAddrSize);

    HeapBuf_Params_init(NULL,&heapbufParams1);

    heapbufParams1.name              = NULL;
    heapbufParams1.sharedAddr        = (Ptr)(curAddr + HEAPBUFMEMOFFSET_2);
    heapbufParams1.align             = 128;
    heapbufParams1.numBlocks         = 4;
    heapbufParams1.blockSize         = MSGSIZE;
    heapbufParams1.gate        = (Gate_Handle)gateHandle;
    heapbufParams1.sharedAddrSize    = HeapBuf_sharedMemReq(&heapbufParams1,
                                        &heapbufParams1.sharedBufSize);
    heapbufParams1.sharedBuf          = (Ptr)(curAddr + HEAPBUFMEMOFFSET_2 +
                                                heapbufParams1.sharedAddrSize);

    size = HeapBuf_sharedMemReq(&heapbufParams1,
                                        &heapbufParams1.sharedBufSize);
    /* Clear the shared area for the new heap instance */
    memset( heapbufParams1.sharedAddr, 0, heapbufParams1.sharedAddrSize);

    do {
        heapbufParams.sharedAddr = (Ptr)(curAddr + HEAPBUFMEMOFFSET_1);
        status =  HeapBuf_open(&heapHandle,&heapbufParams);
    } while(status < 0);

    if (heapHandle == NULL) {
        Osal_printf ("Error in HeapBuf_open [0x%x]\n",status);
    }
    else {
        printf("HeapBuf_open heapHandle:%p\n", heapHandle);
        heapBlock = (Heap_Block *)Heap_alloc(
                                (Heap_Handle)heapHandle,
                                sizeof(Heap_Block),
                                0u);

        HeapBuf_getExtendedStats((Heap_Handle)heapHandle,
                                  &stats);

        Osal_printf("HeapBuf_getExtendedStats NumAllocatedBlocks = %d\n",
                                                    stats.numAllocatedBlocks);

        status = Heap_free((Heap_Handle)heapHandle,
                            heapBlock,
                            sizeof(Heap_Block));
        if (status < 0) {
            Osal_printf ("Error in Heap_free [0x%x]\n",status);
        }
        else {
            Osal_printf("\nHeapBuf block memory free successfully\n");
        }
    }

    heapHandle1 = HeapBuf_create(&heapbufParams1);
    printf("HeapBuf_create heapHandle1:%p\n", heapHandle1);

    Osal_printf ("Halt the Ducati-side application, set wait to 0,\n");
    Osal_printf ("and run it to completion.\n");
    Osal_printf ("Press any key to continue ...\n");
    getchar ();

    return status;
}


/*!
 *  @brief  Function to execute the shutdown for HeapBufApp sample application
 *
 *  @sa     HeapBufApp_callback
 */
Int
HeapBufApp_shutdown (Void)
{
    Int32 status;
    Osal_printf ("Entered HeapBufApp_shutdown\n");

    status = HeapBuf_close (&heapHandle);
    Osal_printf ("HeapBuf_close status: [0x%x]\n", status);

    status = HeapBuf_delete (&heapHandle1);
    Osal_printf ("HeapBuf_delete status: [0x%x]\n", status);

    status = HeapBuf_destroy();
    Osal_printf ("HeapBuf_destroy status: [0x%x]\n", status);

    status = ProcMgr_close (&HeapBufApp_Prochandle);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

#if defined (SYSLINK_USE_SYSMGR)
    SysMgr_destroy ();
#else /* if defined (SYSLINK_USE_SYSMGR) */
    status = ListMPSharedMemory_destroy();
    Osal_printf ("ListMPSharedMemory_destroy status: [0x%x]\n", status);
    
    status = GatePeterson_destroy();
    Osal_printf ("GatePeterson_destroy status: [0x%x]\n", status);
    
    status = SharedRegion_destroy();
    Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);

    status = NameServer_destroy();
    Osal_printf ("NameServer_destroy status: [0x%x]\n", status);

    status = MultiProc_destroy ();
    Osal_printf ("Multiproc_destroy status: [0x%x]\n", status);

    UsrUtilsDrv_destroy ();
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Leaving HeapBufApp_shutdown\n");
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
