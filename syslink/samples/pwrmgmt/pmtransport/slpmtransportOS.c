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
 *  @file   SlpmTransportOS.c
 *
 *  @brief  OS-specific sample application driver module for MessageQ module
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

/* Module headers */
#include <SysMgr.h>

/* Application header */
#include "slpmtransportApp_config.h"

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */

/** ============================================================================
 *  Extern declarations
 *  ============================================================================
 */
/*!
 *  @brief  Function to execute the startup for SlpmTransport sample application
 */
extern Int SlpmTransport_startup (UInt32 notifyAddr, UInt32 sharedAddr);

/*!
 *  @brief  Function to execute the execute for SlpmTransport sample application
 */
extern Int SlpmTransport_execute (Void);

/*!
 *  @brief  Function to execute the shutdown for SlpmTransport sample app
 */
extern Int SlpmTransport_shutdown (Void);


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  ProcMgr handle.
 */
//ProcMgr_Handle SlpmTransport_procMgrHandle = NULL;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
int
main (int argc, char ** argv)
{
    Int status = 0;

    Osal_printf ("PM Transport MPU - AppM3 sample application\n");

    status = SlpmTransport_startup (0, 0);

    if (status >= 0) {
        SlpmTransport_execute ();
    }

    SlpmTransport_shutdown ();

    return 0;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
