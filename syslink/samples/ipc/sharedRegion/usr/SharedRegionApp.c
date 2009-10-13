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
 *  @file   SharedRegionApp.c
 *
 *  @brief  Sample application for SharedRegion module
 *
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
#endif /* if defined(SYSLINK_USE_SYSMGR) */

#include <ProcMgr.h>
#include <ProcDefs.h>
#include <_ProcMgrDefs.h>

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

#define SHAREDMEM               0x98000000
#define SHAREDMEMSIZE           0xF000

/*
#define SHAREDMEM_PHY           0x83f00000
#define SHAREDMEMSIZE           0xF000
*/
ProcMgr_Handle sharedRegionApp_procMgrHandle = NULL;


UInt32 sharedRegionApp_shAddrBase;

UInt32 curAddr;

void * ProcMgrApp_startup ();


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
Int
sharedRegionApp_startup (Void)
{
    Int  status          = 0;
    NameServer_Params    NameServerParams;
    SharedRegion_Config  cfgShrParams;
#if !defined (SYSLINK_USE_SYSMGR)
    MultiProc_Config multiProcConfig;
#else /* if !defined(SYSLINK_USE_SYSMGR) */
    SysMgr_Config config;
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Entered sharedRegionApp_startup\n");

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
    UsrUtilsDrv_setup ();

    NameServerParams.maxNameLen           = MAX_NAME_LENGTH;
    NameServerParams.maxRuntimeEntries = MAX_VALUE_LENGTH;
    NameServerParams.maxValueLen       = MAX_RUNTIMEENTRIES;

    status = NameServer_setup();
    Osal_printf ("NameServer_setup [0x%x]\n", status);
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    cfgShrParams.gateHandle = NULL;
    cfgShrParams.heapHandle = NULL;
    cfgShrParams.maxRegions = 4;
    status = SharedRegion_setup(&cfgShrParams);
    if (status < 0) {
        Osal_printf ("Error in SharedRegion_setup [0x%x]\n", status);
    }

    status = ProcMgr_open (&sharedRegionApp_procMgrHandle,
                           MultiProc_getId("SysM3"));
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }

    status = ProcMgr_translateAddr (sharedRegionApp_procMgrHandle,
                                    (Ptr) &sharedRegionApp_shAddrBase,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM,
                                    ProcMgr_AddrType_SlaveVirt);

    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n", status);
    } else {
        Osal_printf ("\nProcMgr_translateAddr: Slave Virtual Addr:%x\n"
                    "Master Usr Virtual Addr:%x\n",
                    SHAREDMEM, sharedRegionApp_shAddrBase);
    }

    curAddr = sharedRegionApp_shAddrBase;
    SharedRegion_add(0,(Ptr)curAddr, SHAREDMEMSIZE);

    return 0;
}

Int
sharedRegionApp_execute (Void)
{
    UInt32              usrVirtAddress = sharedRegionApp_shAddrBase;
    SharedRegion_SRPtr  srPtr;
    UInt32                 Index = (~0);
    SharedRegion_Info info_set;
    SharedRegion_Info info_get;


    info_set.isValid = 1;
    info_set.base = (Ptr)(usrVirtAddress + SHAREDMEMSIZE);
    info_set.len = SHAREDMEMSIZE;

    usrVirtAddress += 0x100;

    Osal_printf ("usr vitual address   =  [0x%x]\n", usrVirtAddress);
    srPtr = SharedRegion_getSRPtr((Ptr)usrVirtAddress,0);
    Osal_printf ("shared region pointer =  [0x%x]\n", srPtr);
    usrVirtAddress = (UInt32)SharedRegion_getPtr(srPtr);
    Osal_printf ("usr vitual pointer   =  [0x%x]\n", usrVirtAddress);


    usrVirtAddress += 0x200;

    Osal_printf ("usr vitual address   =  [0x%x]\n", usrVirtAddress);
    srPtr = SharedRegion_getSRPtr((Ptr)usrVirtAddress,0);
    Osal_printf ("shared region pointer =  [0x%x]\n", srPtr);
    usrVirtAddress = (UInt32)SharedRegion_getPtr(srPtr);
    Osal_printf ("usr vitual pointer   =  [0x%x]\n", usrVirtAddress);

    Index = SharedRegion_getIndex((Ptr)usrVirtAddress);
    Osal_printf ("usr vitual address = [0x%x] is in"
        " table entry with index = [0x%x]\n", usrVirtAddress, Index);


    SharedRegion_setTableInfo (2, MultiProc_getId("SysM3"), &info_set);
    SharedRegion_getTableInfo (2, MultiProc_getId("SysM3"), &info_get);
    Osal_printf ("usr vitual pointer in table with index 2 is [0x%x]\n"
                 " with size [0x%x]\n", info_get.base, info_get.len);


    usrVirtAddress += 0x6000;

    Osal_printf ("usr vitual address   =  [0x%x]\n", usrVirtAddress);
    srPtr = SharedRegion_getSRPtr((Ptr)usrVirtAddress,0);
    Osal_printf ("usr region pointer =  [0x%x]\n", srPtr);
    usrVirtAddress = (UInt32)SharedRegion_getPtr(srPtr);
    Osal_printf ("usr vitual pointer   =  [0x%x]\n", usrVirtAddress);


    Osal_printf ("Passing an address which is not in any of the SharedRegion"
        " areas registered\n");
    usrVirtAddress += 0x200000;
    Osal_printf ("usr vitual address   =  [0x%x]\n", usrVirtAddress);
    srPtr = SharedRegion_getSRPtr((Ptr)usrVirtAddress,0);
    Osal_printf ("usr region pointer =  [0x%x]\n", srPtr);
    usrVirtAddress = (UInt32)SharedRegion_getPtr(srPtr);
    Osal_printf ("usr vitual pointer   =  [0x%x]\n", usrVirtAddress);
    return (0);
}

Int
sharedRegionApp_shutdown (Void)
{
    Int32 status = 0;

    SharedRegion_remove(0);

    status = SharedRegion_destroy();
    Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);

    status = ProcMgr_close (&sharedRegionApp_procMgrHandle);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

#if defined (SYSLINK_USE_SYSMGR)
    SysMgr_destroy ();
#else /* if defined (SYSLINK_USE_SYSMGR) */
    status = NameServer_destroy();
    Osal_printf ("NameServer_destroy status: [0x%x]\n", status);
    status = MultiProc_destroy ();
    Osal_printf ("Multiproc_destroy status: [0x%x]\n", status);
    UsrUtilsDrv_destroy ();
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Leaving sharedRegionApp_shutdown\n");
    return (0);
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
