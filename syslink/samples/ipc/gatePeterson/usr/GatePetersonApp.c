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
 *  @file   GatePetersonApp.c
 *
 *  @brief  Sample application for GatePeterson module
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
/*
 *  The shared memory is going to split between
 *  Notify:       0xA0000000 - 0xA0004000
 *  HeapBuf:      0xA0004000 - 0xA0008000
 *  Transport:    0xA0008000 - 0xA000A000
 *  GatePeterson: 0xA000A000 - 0xA000B000
 *  MessageQ NS:  0xA000B000 - 0xA000C000
 *  HeapBuf NS:   0xA000C000 - 0xA000D000
 *  NameServer:   0xA000D000 - 0xA000E000
 *  Gatepeterson1:0xA000E000 - 0xA000F000
 */
#define SHAREDMEM               0xA0000000
#define SHAREDMEMSIZE           0xF000

ProcMgr_Handle gatePetersonApp_procMgrHandle = NULL;
UInt32 gatePetersonApp_Handle = 0;
UInt32 gatePetersonApp_shAddr_usr_Base = 0;


UInt32 curAddr;

void GatePeterson_app();
Handle GatePeterson_app_startup();
void GatePeterson_app_execute(Handle gateHandle);
void GatePeterson_app_shutdown(Handle *gateHandle);
void * ProcMgrApp_startup ();

/** ============================================================================
 *  Functions
 *  ============================================================================
 */
int
main (int argc, char ** argv)
{
    Char *trace    = (Char *)TRUE;
    Bool sharedRegionApp_enableTrace = TRUE;
    Char * traceEnter = (Char *)TRUE;
    Bool sharedRegionApp_enableTraceEnter = TRUE;
    Char * traceFailure    = (Char *)TRUE;
    Bool sharedRegionApp_enableTraceFailure = TRUE;
    Char *traceClass = (Char *)TRUE;
    UInt32 sharedRegionApp_traceClass = 0;


    /* Display the version info and created date/time */
    Osal_printf ("GatePeterson sample application created on Date:%s Time:%s\n",
            __DATE__,
            __TIME__);

    trace = getenv ("TRACE");
    /* Enable/disable levels of tracing. */
    if (trace != NULL) {
        Osal_printf ("Trace enable %s\n", trace) ;
        sharedRegionApp_enableTrace = strtol (trace, NULL, 16);
        if ((sharedRegionApp_enableTrace != 0) && (sharedRegionApp_enableTrace != 1)) {
            Osal_printf ("Error! Only 0/1 supported for TRACE\n") ;
        }
        else if (sharedRegionApp_enableTrace == TRUE) {
            curTrace = GT_TraceState_Enable;
        }
        else if (sharedRegionApp_enableTrace == FALSE) {
            curTrace = GT_TraceState_Disable;
        }
    }

    traceEnter = getenv ("TRACEENTER");
    if (traceEnter != NULL) {
        Osal_printf ("Trace entry/leave prints enable %s\n", traceEnter) ;
        sharedRegionApp_enableTraceEnter = strtol (traceEnter, NULL, 16);
        if (    (sharedRegionApp_enableTraceEnter != 0)
            &&  (sharedRegionApp_enableTraceEnter != 1)) {
            Osal_printf ("Error! Only 0/1 supported for TRACEENTER\n") ;
        }
        else if (sharedRegionApp_enableTraceEnter == TRUE) {
            curTrace |= GT_TraceEnter_Enable;
        }
    }

    traceFailure = getenv ("TRACEFAILURE");
    if (traceFailure != NULL) {
        Osal_printf ("Trace SetFailureReason enable %s\n", traceFailure) ;
        sharedRegionApp_enableTraceFailure = strtol (traceFailure, NULL, 16);
        if (    (sharedRegionApp_enableTraceFailure != 0)
            &&  (sharedRegionApp_enableTraceFailure != 1)) {
            Osal_printf ("Error! Only 0/1 supported for TRACEENTER\n");
        }
        else if (sharedRegionApp_enableTraceFailure == TRUE) {
            curTrace |= GT_TraceSetFailure_Enable;
        }
    }

    traceClass = getenv ("TRACECLASS");
    if (traceClass != NULL) {
        Osal_printf ("Trace class %s\n", traceClass);
        sharedRegionApp_traceClass = strtol (traceClass, NULL, 16);
        if (    (sharedRegionApp_enableTraceFailure != 1)
            &&  (sharedRegionApp_enableTraceFailure != 2)
            &&  (sharedRegionApp_enableTraceFailure != 3)) {
            Osal_printf ("Error! Only 1/2/3 supported for TRACECLASS\n");
        }
        else {
            sharedRegionApp_traceClass =
                            sharedRegionApp_traceClass << (32 - GT_TRACECLASS_SHIFT);
            curTrace |= sharedRegionApp_traceClass;
        }
    }

    GatePeterson_app();
    return 0;
}

void GatePeterson_app()
{
    Handle gatepeterson_handle = NULL;
    gatepeterson_handle = GatePeterson_app_startup();
    GatePeterson_app_execute(gatepeterson_handle);
    GatePeterson_app_shutdown(&gatepeterson_handle);

}

Handle GatePeterson_app_startup()
{
    Int32 status = 0 ;
    GatePeterson_Config  cfgGateParams;
    GatePeterson_Params gateParams;
    GatePeterson_Handle gateHandle = NULL;
    NameServer_Params NameServerParams;
#if !defined (SYSLINK_USE_SYSMGR)
    SharedRegion_Config  cfgShrParams;
    MultiProc_Config multiProcConfig;
#else /* if !defined(SYSLINK_USE_SYSMGR) */
    SysMgr_Config config;
#endif /* if !defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Entering GatePeterson Application Startup\n");

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

    /*    Below function is called with out opening driver, why?
        NameServer_Params_init(&NameServerParams);
    */
    NameServerParams.maxNameLen        = MAX_NAME_LENGTH;
    NameServerParams.maxRuntimeEntries = MAX_VALUE_LENGTH;
    NameServerParams.maxValueLen       = MAX_RUNTIMEENTRIES;
    NameServer_setup();
    Osal_printf ("NameServer_setup [0x%x]\n", status);

    cfgShrParams.gateHandle = NULL;
    cfgShrParams.heapHandle = NULL;
    cfgShrParams.maxRegions = 256;
    status = SharedRegion_setup(&cfgShrParams);
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    GatePeterson_getConfig (&cfgGateParams);
    cfgGateParams.maxNameLen = MAX_NAME_LENGTH;
    cfgGateParams.useNameServer = 1;
    status = GatePeterson_setup (&cfgGateParams);
    if (status < 0) {
        Osal_printf ("Error in GatePeterson_setup [0x%x]\n", status);
    }



    status = ProcMgr_open (&gatePetersonApp_procMgrHandle,
                           MultiProc_getId("SysM3"));
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }

    status = ProcMgr_translateAddr (gatePetersonApp_procMgrHandle,
                                (Ptr) &gatePetersonApp_shAddr_usr_Base,
                                ProcMgr_AddrType_MasterUsrVirt,
                                (Ptr) SHAREDMEM,
                                ProcMgr_AddrType_SlaveVirt);
    SharedRegion_add(0,(Ptr) gatePetersonApp_shAddr_usr_Base, SHAREDMEMSIZE);

    GatePeterson_Params_init(gateHandle, &gateParams);
    gateParams.sharedAddr      = (Ptr)gatePetersonApp_shAddr_usr_Base;
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq(&gateParams);
    gateParams.name          = Memory_alloc(NULL,
                                      sizeof(10),
                                          0u);
    String_ncpy(gateParams.name,"TESTGATE",String_len ("TESTGATE"));

    gateHandle = GatePeterson_create(&gateParams);
    if (gateHandle == NULL) {
         Osal_printf ("Error in GatePeterson_create \n");
    }

    Osal_printf ("Leaving GatePeterson Application Startup\n");
    return gateHandle;
}

void GatePeterson_app_execute(Handle gateHandle)
{
    UInt32 key = 0;
    Osal_printf ("Entering GatePeterson Application Execute\n");
    key = GatePeterson_enter(gateHandle);
    GatePeterson_leave(gateHandle, key);
    Osal_printf ("Leaving GatePeterson Application Execute\n");
}

void GatePeterson_app_shutdown(Handle *gateHandle)
{
    Int32 status = 0 ;

    Osal_printf ("Entering GatePeterson Application Shutdown\n");

    status = GatePeterson_delete((GatePeterson_Handle *)gateHandle);
    if (gateHandle < 0) {
        Osal_printf ("Error in GatePeterson_delete [0x%x]\n", status);
    }

    status = GatePeterson_destroy();
    Osal_printf ("GatePeterson_destroy status [0x%x]\n", status);

    status = ProcMgr_close (&gatePetersonApp_procMgrHandle);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);

#if defined (SYSLINK_USE_SYSMGR)
    SysMgr_destroy ();
#else /* if defined (SYSLINK_USE_SYSMGR) */
    status = SharedRegion_destroy();
    Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);
    status = NameServer_destroy();
    Osal_printf ("NameServer_destroy status: [0x%x]\n", status);
    status = MultiProc_destroy ();
    Osal_printf ("Multiproc_destroy status: [0x%x]\n", status);
    UsrUtilsDrv_destroy ();
#endif /* if defined(SYSLINK_USE_SYSMGR) */

    Osal_printf ("Leaving GatePeterson Application Execute\n");
}
