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
 *  @file   TilerSyslinkApp.c
 *
 *  @brief  The Syslink Test sample to validate Syslink Mem utils functinalit
 *
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
#include <stdint.h>
#include <time.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>
#include <String.h>
#include <Trace.h>

/* Tiler header file */
#include <tilermgr.h>

/* IPC headers */
#include <SysMgr.h>
#include <ProcMgr.h>
#include <SysLinkMemUtils.h>


/* RCM headers */
#include <RcmClient.h>

/* Sample headers */
#include <Client.h>
#include <MemAllocTest_Config.h>
#include "TilerSyslinkApp.h"

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */


/* =============================================================================
 * Structs & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining RCM remote function arguments
 */
typedef struct {
    UInt numBytes;
     /*!< Size of the Buffer */
    Ptr bufPtr;
     /*!< Buffer that is passed */
} RCM_Remote_FxnArgs;

 /* ===========================================================================
 *  APIs
 * ============================================================================
 */
/*!
 *  @brief       Function to Test use buffer functionality using Tiler and
 *               Syslink IPC
 *
 *  @param procId       The Proc ID with which this functionality is verified
 *  @param useTiler     Flag to enable TILER allocation
 *  @param numTrials    Number of times to run use buffer test
 *
 *  @sa
 */
Int SyslinkUseBufferTest (Int procId, Bool useTiler, UInt numTrials)
{
    Int                             fd;
    void *                          mapBase;
    SyslinkMemUtils_MpuAddrToMap    MpuAddr_list[1];
    UInt32                          mapSize = 4096;
    UInt32                          mappedAddr;
    SysMgr_Config                   config;
    Int                             status = 0;
    Int                             procIdSysM3 = PROC_SYSM3;
    Int                             procIdAppM3 = PROC_APPM3;
#if defined (SYSLINK_USE_LOADER)
    Char *                          imageNameSysM3;
    Char *                          imageNameAppM3;
    UInt32                          fileIdSysM3;
    UInt32                          fileIdAppM3;
#endif
    ProcMgr_StartParams             start_params;
    ProcMgr_StopParams              stop_params;
    ProcMgr_Handle                  procMgrHandle_client;
    RcmClient_Handle                rcmClientHandle = NULL;
    RcmClient_Config                cfgParams;
    RcmClient_Params                rcmClient_Params;
    Char *                          remoteServerName;

    UInt                            fxnBufferTestIdx;
    UInt                            fxnExitIdx;

    RcmClient_Message *             rcmMsg = NULL;
    UInt                            rcmMsgSize;
    RCM_Remote_FxnArgs *            fxnArgs;
    Int                             count = 0;
    Int                             maxCount = 3;
    UInt32                          entry_point = 0;
    UInt                            i;
    Ptr                             bufPtr = NULL;
    UInt                            usrSharedAddr;
    ProcMgr_MapType                 mapType;
    UInt                            k;
    UInt *                          uintBuf;

    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }

    printf("RCM procId= %d\n", procId);

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle_client,
                           procIdSysM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        Osal_printf ("ProcMgr_open Status [0x%x]\n", status);

        status = ProcMgr_translateAddr (procMgrHandle_client,
                                        (Ptr) &usrSharedAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);

        status = SharedRegion_add (0,
                           (Ptr) usrSharedAddr,
                           SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
        else {
            Osal_printf ("SharedRegion_add [0x%x]\n", status);
        }

        status = ProcMgr_translateAddr (procMgrHandle_client,
                                        (Ptr) &usrSharedAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM1,
                                        ProcMgr_AddrType_SlaveVirt);

        status = SharedRegion_add (1,
                           (Ptr) usrSharedAddr,
                           SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
        else {
            Osal_printf ("SharedRegion_add [0x%x]\n", status);
        }

#if defined (SYSLINK_USE_LOADER)
        imageNameSysM3 = "./MemAllocServer_MPUSYS_Test_Core0.xem3";
        Osal_printf ("loading the image %s\n", imageNameSysM3);
        Osal_printf ("procId = %d\n", procIdSysM3);

        status = ProcMgr_load (procMgrHandle_client, imageNameSysM3, 2,
                                &imageNameSysM3, &entry_point, &fileIdSysM3, procIdSysM3);
#endif
        start_params.proc_id = procIdSysM3;
        Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
        status  = ProcMgr_start(procMgrHandle_client, entry_point, &start_params);
        Osal_printf ("ProcMgr_start Status [0x%x]\n", status);

        if(procId == procIdAppM3) {
#if defined (SYSLINK_USE_LOADER)
            imageNameAppM3 = "./MemAllocServer_MPUAPP_Test_Core1.xem3";
            Osal_printf ("loading the image %s\n", imageNameAppM3);
            Osal_printf ("procId = %d\n", procIdAppM3);

            status = ProcMgr_load (procMgrHandle_client, imageNameAppM3, 2,
                                    &imageNameAppM3, &entry_point, &fileIdAppM3, procIdAppM3);
#endif
            start_params.proc_id = procIdAppM3;
            Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
            status  = ProcMgr_start(procMgrHandle_client, entry_point, &start_params);
            Osal_printf ("ProcMgr_start Status [0x%x]\n", status);
        }
    }

    ///////////////////////// Set up RCM /////////////////////////

    /* Get default config for rcm client module */
    Osal_printf ("Get default config for rcm client module.\n");
    status = RcmClient_getConfig(&cfgParams);
    if (status < 0) {
        Osal_printf ("Error in RCM Client module get config \n");
        goto exit;
    } else {
        Osal_printf ("RCM Client module get config passed \n");
    }

    cfgParams.defaultHeapBlockSize = MSGSIZE;

    /* rcm client module setup*/
    Osal_printf ("RCM Client module setup.\n");
    status = RcmClient_setup (&cfgParams);
    if (status < 0) {
        Osal_printf ("Error in RCM Client module setup \n");
        goto exit;
    } else {
        Osal_printf ("RCM Client module setup passed \n");
    }

    /* rcm client module params init*/
    Osal_printf ("RCM Client module params init.\n");
    status = RcmClient_Params_init(NULL, &rcmClient_Params);
    if (status < 0) {
        Osal_printf ("Error in RCM Client instance params init \n");
        goto exit;
    } else {
        Osal_printf ("RCM Client instance params init passed \n");
    }

    if(procId == procIdSysM3) {
        remoteServerName = RCM_SERVER_NAME_SYSM3;
        rcmClient_Params.heapId = HEAPID_SYSM3;
    }
    else {
        remoteServerName = RCM_SERVER_NAME_APPM3;
        rcmClient_Params.heapId = HEAPID_APPM3;
    }
    /* create an rcm client instance */
    Osal_printf ("Creating RcmClient instance %s.\n", remoteServerName);
    rcmClient_Params.callbackNotification = 0; /* disable asynchronous exec */

    while ((rcmClientHandle == NULL) && (count++ < MAX_CREATE_ATTEMPTS)) {
        status = RcmClient_create (remoteServerName, &rcmClient_Params,
                                    &rcmClientHandle);
        if (status < 0) {
            if (status == RCMCLIENT_ESERVER) {
                Osal_printf ("Unable to open remote server %d time\n", count);
            }
            else {
                Osal_printf ("Error in RCM Client create \n");
                goto exit;
            }
        } else {
            Osal_printf ("RCM Client create passed \n");
        }
    }
    if (MAX_CREATE_ATTEMPTS <= count) {
        Osal_printf ("Timeout... could not connect with remote server\n");
    }


    Osal_printf ("\nQuerying server for fxnBufferTest() function index \n");

    status = RcmClient_getSymbolIndex (rcmClientHandle, "fxnBufferTest",
                                            &fxnBufferTestIdx);
    if (status < 0)
        Osal_printf ("Error getting symbol index [0x%x]\n", status);
    else
        Osal_printf ("fxnBufferTest() symbol index [0x%x]\n", fxnBufferTestIdx);

    Osal_printf ("\nQuerying server for fxnExit() function index \n");

    status = RcmClient_getSymbolIndex (rcmClientHandle, "fxnExit",
                                            &fxnExitIdx);
    if (status < 0)
        Osal_printf ("Error getting symbol index [0x%x]\n", status);
    else
        Osal_printf ("fxnExit() symbol index [0x%x]\n", fxnExitIdx);

    for(k = 0; k < numTrials; k++)
    {
        /////////////////////////// Allocate & map buffer ////////////////////////
        if(useTiler) {
            TilerMgr_Open();
            Osal_printf("Calling tilerAlloc.\n");
            bufPtr = (Ptr)TilerMgr_Alloc(PIXEL_FMT_8BIT, mapSize, 1);
            if(bufPtr == NULL) {
                Osal_printf("Error: tilerAlloc returned null.\n");
                status = -1;

                return status;
            }
            else {
                Osal_printf("tilerAlloc returned 0x%x.\n", (UInt)bufPtr);
            }

            mapType = ProcMgr_MapType_Tiler;
        }
        else {
            Osal_printf("Calling malloc.\n");
            bufPtr = (Ptr)malloc(mapSize);

            if(bufPtr == NULL) {
                Osal_printf("Error: malloc returned null.\n");
                return -1;
            }
            else {
                Osal_printf("malloc returned 0x%x.\n", (UInt)bufPtr);
            }
            mapType = ProcMgr_MapType_Virt;
        }

        if(useTiler) {
            Osal_printf("Opening /dev/mem.\n");
            fd = open ("/dev/mem", O_RDWR|O_SYNC);
            if (fd) {
                mapBase = mmap(0,  mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                                                           (UInt)bufPtr);
                if(mapBase == (void *) -1) {
                    Osal_printf("Failed to do memory mapping \n");
                    return -1;
                }
            }
            else {
                Osal_printf("Failed opening /dev/mem file\n");
                return -2;
            }
        }
        else
            mapBase = bufPtr;

        printf("map_base = 0x%x \n", (UInt32)mapBase);
        MpuAddr_list[0].mpuAddr = (UInt32)mapBase;
        MpuAddr_list[0].size = mapSize;
        status = SysLinkMemUtils_map (MpuAddr_list, 1, &mappedAddr,
                                mapType, PROC_SYSM3);
        Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x\n",
                                MpuAddr_list[0].mpuAddr, mappedAddr);

        //////////////// Do actual test here ////////////////

        uintBuf = (UInt*)mapBase;
        for(i = 0; i < mapSize / sizeof(UInt); i++) {
            uintBuf[i] = 0;
            uintBuf[i] = (0xbeef0000 | i);

            if(uintBuf[i] != (0xbeef0000 | i)) {
                Osal_printf("Readback failed at address 0x%x\n", &uintBuf[i]);
                Osal_printf("\tExpected: [0x%x]\tActual: [0x%x]\n", (0xbeef0000 | i), uintBuf[i]);
            }
        }

        // allocate a remote command message
        Osal_printf("Allocating RCM message\n");
        rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
        rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
        if (rcmMsg == NULL) {
            Osal_printf("Error allocating RCM message\n");
            goto exit;
        }

        // fill in the remote command message
        rcmMsg->fxnIdx = fxnBufferTestIdx;
        fxnArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
        fxnArgs->numBytes = mapSize;
        fxnArgs->bufPtr   = (Ptr)mappedAddr;

        status = RcmClient_exec (rcmClientHandle, rcmMsg);

        if (status < 0) {
            Osal_printf (" RcmClient_exec error. \n");
        }
        else {
            // Check the buffer data
            Osal_printf ("Testing data\n");
            count = 0;
            for(i = 0; i < mapSize / sizeof(UInt) && count < maxCount; i++) {
                if(uintBuf[i] != ~(0xbeef0000 | i)) {
                    Osal_printf("ERROR: Data mismatch at offset 0x%x\n", i * sizeof(UInt));
                    Osal_printf("\tExpected: [0x%x]\tActual: [0x%x]\n", ~(0xbeef0000 | i), uintBuf[i]);
                    count ++;
                }
            }

            if(count == 0)
                Osal_printf("Test passed!\n");
        }

        // Set the memory to some other value to avoid a
        // potential future false positive
        for(i = 0; i < mapSize / sizeof(UInt); i++) {
            uintBuf[i] = 0xdeadbeef;
        }

        // return message to the heap
        Osal_printf ("Calling RcmClient_free\n");
        RcmClient_free (rcmClientHandle, rcmMsg);

        ///////////////////// Cleanup //////////////////////

        if(useTiler) {
            Osal_printf("Freeing TILER buffer\n");
            TilerMgr_Free((Int)bufPtr);

            if (munmap(mapBase,mapSize) == -1)
                    Osal_printf("Memory Unmap failed.\n");
            else
                Osal_printf("Memory Unmap successful.\n");
            close(fd);

            TilerMgr_Close();
        }
        else {
            SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
            free(bufPtr);
        }
    }

    //////////////////////// Shutdown RCM /////////////////////

    // allocate a remote command message
    rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
    rcmMsg = RcmClient_alloc (rcmClientHandle, rcmMsgSize);
    if (rcmMsg == NULL) {
        Osal_printf ("Error allocating RCM message\n");
        goto exit;
    }

    // fill in the remote command message
    rcmMsg->fxnIdx = fxnExitIdx;
    fxnArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);

    // execute the remote command message
    Osal_printf ("calling RcmClient_execDpc \n");
    status = RcmClient_execDpc (rcmClientHandle, rcmMsg);
    if (status < 0) {
        Osal_printf ("RcmClient_execDpc error. \n");
        goto exit;
    }

    // return message to the heap
    Osal_printf ("calling RcmClient_free \n");
    RcmClient_free (rcmClientHandle, rcmMsg);

    /* delete the rcm client */
    Osal_printf ("Delete RCM client instance \n");
    status = RcmClient_delete (&rcmClientHandle);
    if (status < 0) {
        Osal_printf ("Error in RCM Client instance delete\n");
    }

    /* rcm client module destroy*/
    Osal_printf ("Destroy RCM client module \n");
    status = RcmClient_destroy ();
    if (status < 0) {
        Osal_printf ("Error in RCM Client module destroy \n");
    }

    /* Finalize modules */
    SharedRegion_remove (0);
    SharedRegion_remove (1);

    if(procId == procIdAppM3) {
        stop_params.proc_id = procIdAppM3;
        status = ProcMgr_stop(procMgrHandle_client, &stop_params);
        Osal_printf("ProcMgr_stop status: [0x%x]\n", status);
    }

    stop_params.proc_id = procIdSysM3;
    status = ProcMgr_stop(procMgrHandle_client, &stop_params);
    Osal_printf("ProcMgr_stop status: [0x%x]\n", status);


    status = ProcMgr_close (&procMgrHandle_client);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

    status = SysMgr_destroy();
    Osal_printf("SysMgr_destroy status: [0x%x]\n", status);

    Osal_printf ("SyslinkUseBufferTest done!\n");

exit:
    return status;

}

/*!
 *  @brief       Function to test the retrieval of Pages for
 *               both Tiler and non-Tiler buffers
 *
 *  @param     void
 *
 *  @sa
 */
Int SyslinkVirtToPhysPagesTest(void)
{
    Int status = 0;
    UInt32 remoteAddr;
    Int numOfIterations = 1;
    UInt32 physEntries[10];
    UInt32 NumOfPages = 10;
    UInt32 temp;
    SysMgr_Config                   config;
/*
    int *p;
    UInt32 mappedAddr;
    SyslinkMemUtils_MpuAddrToMap MpuAddr_list[1];
    UInt32 sizeOfBuffer = 0x1000;
*/

    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    Osal_printf ("Testing SyslinkVirtToPhysTest\n");
    remoteAddr = 0x60000000;
    do {
        status = SysLinkMemUtils_virtToPhysPages (remoteAddr, NumOfPages, physEntries,
                                            PROC_SYSM3);
        if (status < 0)
        {
            Osal_printf("SysLinkMemUtils_virtToPhysPages failure,status"
                        " = 0x%x\n",(UInt32)status);
            return status;
        }
        for (temp = 0; temp < NumOfPages; temp++)
        {
            Osal_printf("remoteAddr = [0x%x]  physAddr = [0x%x]\n", remoteAddr,
                                                            physEntries[temp]);
            remoteAddr += 4096;
        }
        numOfIterations--;
    }while (numOfIterations > 0);
/*
    p = (int *)malloc(sizeOfBuffer);
    MpuAddr_list[0].mpuAddr = (UInt32)p;
    MpuAddr_list[0].size = sizeOfBuffer + 0x1000;
    status = SysLinkMemUtils_map (MpuAddr_list, 1, &mappedAddr,
                            ProcMgr_MapType_Virt, PROC_SYSM3);
    mappedAddr = (UInt32)p;
    NumOfPages = 3;
    status = SysLinkMemUtils_virtToPhysPages (mappedAddr, NumOfPages, physEntries,
                                            PROC_SYSM3);
    for (temp = 0; temp < NumOfPages; temp++)
    {
        Osal_printf("remoteAddr = [0x%x]  physAddr = [0x%x]\n",
                    (mappedAddr + (temp*4096)), physEntries[temp]);
    }
    SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
    free(p);
*/
    SysMgr_destroy();
    return status;
}


/*!
 *  @brief       Function to test the retrieval phsical address for
 *               a given Co-Processor virtual address.
 *
 *  @param     void
 *
 *  @sa
 */
Int SyslinkVirtToPhysTest(void)
{
    UInt32 remoteAddr;
    UInt32 physAddr;
    Int numOfIterations = 10;

    Osal_printf ("Testing SyslinkVirtToPhysTest\n");
    remoteAddr = 0x60000000;
    do {
        SysLinkMemUtils_virtToPhys (remoteAddr, &physAddr, PROC_SYSM3);
        Osal_printf("remoteAddr = [0x%x]  physAddr = [0x%x]\n", remoteAddr,
                                                            physAddr);
        remoteAddr += 4096;
        numOfIterations--;
    }while (numOfIterations > 0);
    return 0;
}

/*!
 *  @brief       Function to test multiple calls to map/unmap
 *
 *  @param   numTrials  Number of times to call map/unmap
 *
 *  @sa
 */
Int SyslinkMapUnMapTest(UInt numTrials)
{
    Ptr                             bufPtr;
    UInt                            bufSize;
    UInt                            i;
    SyslinkMemUtils_MpuAddrToMap    MpuAddr_list[1];
    UInt32                          mappedAddr;
    Int                             status = 0;
    SysMgr_Config                   config;
    ProcMgr_Handle                  procMgrHandle_client;
    UInt                            usrSharedAddr;

    // Randomize
    srand(time(NULL));

    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle_client,
                           PROC_SYSM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        Osal_printf ("ProcMgr_open Status [0x%x]\n", status);

        status = ProcMgr_translateAddr (procMgrHandle_client,
                                        (Ptr) &usrSharedAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);

        status = SharedRegion_add (0,
                           (Ptr) usrSharedAddr,
                           SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
        else {
            Osal_printf ("SharedRegion_add [0x%x]\n", status);
        }

        status = ProcMgr_translateAddr (procMgrHandle_client,
                                        (Ptr) &usrSharedAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM1,
                                        ProcMgr_AddrType_SlaveVirt);

        status = SharedRegion_add (1,
                           (Ptr) usrSharedAddr,
                           SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
        }
        else {
            Osal_printf ("SharedRegion_add [0x%x]\n", status);
        }

        for(i = 0; i < numTrials; i++) {

            // Generate random size to allocate (up to 64K)
            bufSize = (rand() & 0xFFFF);
            if(bufSize == 0)
                bufSize = 1;

            Osal_printf("Calling malloc with size %d.\n", bufSize);
            bufPtr = (Ptr)malloc(bufSize);

            if(bufPtr == NULL) {
                Osal_printf("Error: malloc returned null.\n");
                return -1;
            }
            else {
                Osal_printf("malloc returned 0x%x.\n", (UInt)bufPtr);
            }

            MpuAddr_list[0].mpuAddr = (UInt32)bufPtr;
            MpuAddr_list[0].size = bufSize;
            status = SysLinkMemUtils_map (MpuAddr_list, 1, &mappedAddr,
                                    ProcMgr_MapType_Virt, PROC_SYSM3);
            if(status < 0) {
                Osal_printf("SysLinkMemUtils_map failed with status [0x%x].\n", status);
                return -2;
            }
            Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x\n",
                                    MpuAddr_list[0].mpuAddr, mappedAddr);

            status = SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
            if(status < 0) {
                Osal_printf("SysLinkMemUtils_unmap failed with status [0x%x].\n", status);
                return -3;
            }

            free(bufPtr);
        }
    }

    /* Finalize modules */
    SharedRegion_remove (0);
    SharedRegion_remove (1);

    status = ProcMgr_close (&procMgrHandle_client);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

    status = SysMgr_destroy();
    Osal_printf("SysMgr_destroy status: [0x%x]\n", status);

    Osal_printf("Map/UnMap test passed!\n");
    return 0;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
