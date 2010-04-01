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
 *  @file   ListMPApp.c
 *
 *  @brief  Sample application for ListMP module
 *  ============================================================================
 */

/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
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
#include <ProcMgr.h>
#include <NameServer.h>
#include <SharedRegion.h>
#include <GatePeterson.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>
#endif /* if defined(SYSLINK_USE_SYSMGR) */
#include <omap4430proc.h>
#include <ConfigNonSysMgrSamples.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
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

/*!
 *  @brief  shared memory size
 */
#define SHAREDMEM               0xA0000000
#define SHAREDMEMSIZE           0x1B000

/* Memory for the Notify Module */
#define NOTIFYMEM               (SHAREDMEM)
#define NOTIFYMEMSIZE           0x4000

/* Memory a GatePeterson instance */
#define GATEPETERSONMEM         (NOTIFYMEM + NOTIFYMEMSIZE)
#define GATEPETERSONMEMSIZE     0x1000


/* Memory a HeapMultiBuf instance */
#define HEAPMBMEM_CTRL          (GATEPETERSONMEM + GATEPETERSONMEMSIZE)
#define HEAPMBMEMSIZE_CTRL      0x1000
#define HEAPMBMEM_BUFS          (HEAPMBMEM_CTRL + HEAPMBMEMSIZE_CTRL)
#define HEAPMBMEMSIZE_BUFS      0x3000

/* Memory for NameServerRemoteNotify */
#define NSRN_MEM                (HEAPMBMEM_BUFS + HEAPMBMEMSIZE_BUFS)
#define NSRN_MEMSIZE            0x1000

/* Memory a Transport instance */
#define TRANSPORTMEM            (NSRN_MEM + NSRN_MEMSIZE)
#define TRANSPORTMEMSIZE        0x2000

/* Memory for MessageQ's NameServer instance */
#define MESSAGEQ_NS_MEM         (TRANSPORTMEM + TRANSPORTMEMSIZE)
#define MESSAGEQ_NS_MEMSIZE     0x1000

/* Memory for HeapBuf's NameServer instance */
#define HEAPBUF_NS_MEM          (MESSAGEQ_NS_MEM + MESSAGEQ_NS_MEMSIZE)
#define HEAPBUF_NS_MEMSIZE      0x1000

#define GATEPETERSONMEM1        (HEAPBUF_NS_MEM + HEAPBUF_NS_MEMSIZE)
#define GATEPETERSONMEMSIZE1    0x1000

/* Memory for the Notify Module */
#define HEAPMEM                 (GATEPETERSONMEM1 + GATEPETERSONMEMSIZE1)
#define HEAPMEMSIZE             0x1000

#define HEAPMEM1                (HEAPMEM + HEAPMEMSIZE)
#define HEAPMEMSIZE1            0x1000

#define List                    (HEAPMEM1 + HEAPMEMSIZE1)
#define ListSIZE                0x1000

#define List1                   (List + ListSIZE)
#define ListSIZE1               0x1000

#define LISTMP_OFFSET           (List - SHAREDMEM)
#define LISTMP1_OFFSET          (List1 - SHAREDMEM)
#define GATEPETERSON_OFFSET     (GATEPETERSONMEM - SHAREDMEM)

#if defined(SYSLINK_USE_SYSMGR)
#define LISTMP_SYSM3_IMAGE_PATH "/binaries/ListMPTestApps_SYSM3_MPU_SYSMGR.xem3"
#else
#define LISTMP_SYSM3_IMAGE_PATH "/binaries/ListMPTestApps_SYSM3_MPU_NONSYSMGR.xem3"
#endif
//#define LISTMP_SYSM3_IMAGE_PATH "/binaries/Notify_MPUSYS_reroute_Test_Core0.xem3"
#define LISTMP_APPM3_IMAGE_PATH ""

/*
 * POSSIBLE TEST CASES
 * 1.  create and open the list on same processor and try list operations
 * 2.  create on DSP insert element from GPP
 * 3.  remove element when only one / no element in the list
 * 4.  keep inserting nodes till allocated shared region limit crosses. Try
 *     putHead, putail and insert after this
 *       - we should put ckeck for this.
 * 5.  open the list and try to delete it. (It should give error)
 * 6.  open list simultaneously on two or more processors
 * 7.  create list simultaneously on two or more processors with same sharedAddr
 * 8.  Try next/prev after getHead/getTail of list with only one node.
 * 9.  Try remove from empty list
 * 10. list delete after list destroy.
 */


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Handle to the ListMP instance used.
 */
ListMPSharedMemory_Handle ListMPApp_handle;
GatePeterson_Handle       ListMPApp_gateHandle;
UInt32  ListMPApp_shAddrBase;

typedef struct ListMP_Node_Tag
{
    ListMP_Elem elem;
    Int32  id;
}ListMP_Node;

typedef struct ListMP_Node1_Tag
{
    ListMP_Elem elem;
    Int32  id;
}ListMP_Node1;


Int
ListMPApp_startup (UInt32 sharedAddr)
{
    Int                       status  = 0;
#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_Config config;
#else
    SharedRegion_Config       cfgShrParams;
    GatePeterson_Config       gpConfig;
    NameServer_Params         nameServerParams;
    ListMPSharedMemory_Config cfgLstParams;
#endif

    Osal_printf ("\nEntered ListMPApp_startup\n");

#if defined(SYSLINK_USE_SYSMGR)
    SysMgr_getConfig (&config);
    Osal_printf ("Calling SysMgr_setup \n");
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
    }
    status = ProcMgr_open (&procHandle,
                           MultiProc_getId("SysM3"));
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
#else /* if defined(SYSLINK_USE_SYSMGR) */
    UsrUtilsDrv_setup ();
#endif

    procId = 2;

#if !defined(SYSLINK_USE_SYSMGR)
    cfgShrParams.gateHandle = NULL;
    cfgShrParams.heapHandle = NULL;
    cfgShrParams.maxRegions = 4;

    /* SharedRegion module setup */
    status = SharedRegion_setup (&cfgShrParams);
    if (status < 0) {
        Osal_printf ("Error in SharedRegion_setup [0x%x]\n", status);
    }
    else {
        Osal_printf ("SharedRegion_setup status [0x%x]\n", status);
    }
    Osal_printf ("SharedRegion_setup successfully\n");

    ProcUtil_setup ();
    Osal_printf("ProcUtil_setup done\n");

    NameServer_setup ();
#endif

    status = ProcMgr_translateAddr (procHandle,
                                    (Ptr) &ListMPApp_shAddrBase,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) SHAREDMEM,
                                    ProcMgr_AddrType_SlaveVirt);

    sharedAddr = ListMPApp_shAddrBase;
    Osal_printf ("After ProcMgr_translateAddr sharedAddr=%x\n",sharedAddr);

    if (status < 0) {
        Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n", status);
    }
    SharedRegion_add (0, (Ptr) sharedAddr, SHAREDMEMSIZE);

#if !defined(SYSLINK_USE_SYSMGR)
    NameServer_Params_init (&nameServerParams);
    nameServerParams.maxNameLen        = MAX_NAME_LENGTH;
    nameServerParams.maxRuntimeEntries = MAX_VALUE_LENGTH;
    nameServerParams.maxValueLen       = MAX_RUNTIMEENTRIES;

    ListMPSharedMemory_getConfig(&cfgLstParams);
    cfgLstParams.maxNameLen = MAX_NAME_LENGTH;

    if (status >= 0)
    {
        /* ListMPSharedMemory module setup */
        status = ListMPSharedMemory_setup(&cfgLstParams);
        if (status < 0) {
            Osal_printf("Error in ListMPSharedMemory_setup [0x%x]\n", status);
        }
        else {
            Osal_printf ("ListMPSharedMemory_setup status [0x%x]\n", status);
        }
    }

    if (status >= 0) {
        /* GatePeterson module setup */
        GatePeterson_getConfig (&gpConfig);
        status = GatePeterson_setup (&gpConfig);
        if (status < 0) {
            Osal_printf ("Error in GatePeterson_setup. Status [0x%x]\n",
                         status);
        }
        else {
            Osal_printf ("GatePeterson_setup status [0x%x]\n", status);
        }
    }
#endif

    ProcUtil_load (LISTMP_SYSM3_IMAGE_PATH);
    Osal_printf ("Done loading image to SYSM3\n");
    Osal_printf ("Leaving ListMPApp_startup\n");

    return (status);
}


Int
ListMPApp_execute (UInt32 sharedAddr)
{

    Int                         status  = -1;
    ListMPSharedMemory_Params   listMPParams;
    ListMPSharedMemory_Params   listMPParams1;
    ListMP_Handle               listHandle;
    ListMP_Handle               listHandle1;
    ListMP_Node *               node;
    GatePeterson_Params         gateParams;
    UInt32                      listAddr;
    UInt32                      list1Addr;
    UInt32                      gateAddr;

    sharedAddr = ListMPApp_shAddrBase;
    
    Osal_printf ("\nEntered ListMPApp_execute\n");

    // Compute locations in user memory
    listAddr    = sharedAddr + LISTMP_OFFSET;
    list1Addr   = sharedAddr + LISTMP1_OFFSET;
    gateAddr    = sharedAddr + GATEPETERSON_OFFSET;

    procId = 2;
    ProcUtil_start ();
    Osal_printf ("Started Ducati:SYSM3\n");

    GatePeterson_Params_init (ListMPApp_gateHandle, &gateParams);
    gateParams.sharedAddrSize = GatePeterson_sharedMemReq (&gateParams);
    Osal_printf ("Memory required for GatePeterson instance [0x%x]"
                 " bytes \n",
                 gateParams.sharedAddrSize);
    do {
        gateParams.sharedAddr     = (Ptr)(gateAddr);
        status = GatePeterson_open (&ListMPApp_gateHandle,
                                    &gateParams);
    }
    while ((status == GATEPETERSON_E_NOTFOUND) ||
            (status == GATEPETERSON_E_VERSION));
    if (status < 0) {
        Osal_printf ("Error in GatePeterson_open [0x%x]\n", status);
    }
    else {
        Osal_printf ("GatePeterson_open status [0x%x]\n", status);
    }

    if(status >= 0) {
        ListMPSharedMemory_Params_init(NULL,&listMPParams);
        listMPParams.gate            = ListMPApp_gateHandle;
        listMPParams.sharedAddr     = (Ptr)listAddr;
        listMPParams.listType       = ListMP_Type_SHARED;
        listMPParams.sharedAddrSize = ListMP_sharedMemReq(&listMPParams);

        ListMPSharedMemory_Params_init(NULL,&listMPParams1);
        listMPParams1.gate             = ListMPApp_gateHandle;
        listMPParams1.sharedAddr     = (Ptr)list1Addr;
        listMPParams1.listType       = ListMP_Type_SHARED;
        listMPParams1.sharedAddrSize = ListMP_sharedMemReq(&listMPParams1);

        Osal_printf("Creating ListMP at 0x%08x\n", list1Addr);
        
        listHandle1 = ListMP_create(&listMPParams1);

        // Get next unused shared memory
        sharedAddr = list1Addr + listMPParams1.sharedAddrSize;

        node = (ListMP_Node *) (sharedAddr + 64);
        node->id = 100;

        ListMP_putTail((ListMP_Handle)listHandle1,&(node->elem));
    }

    if(status >= 0) {
        Osal_printf ("Opening the list created by remote processor at 0x%08x\n", listAddr);
        status = ListMP_open(&listHandle,&listMPParams);
        if ((status < 0) || (listHandle == NULL)) {
            Osal_printf ("Error in ListMP_open [0x%x]\n", status);
        }
        else {
            Osal_printf ("List opened successfully\n");
            node = (ListMP_Node *)ListMP_getHead(listHandle);
            Osal_printf ("List Head element  ID = %d\n",node->id);
            node = (ListMP_Node *) (sharedAddr + 64);
            node->id = 111;
            Osal_printf ("List PutTail       ID = %d\n",node->id);
            ListMP_putTail((ListMP_Handle)listHandle,&(node->elem));
            node = (ListMP_Node *) (sharedAddr + 128);
            node->id = 333;
            Osal_printf ("List PutTail       ID = %d\n",node->id);
            ListMP_putTail((ListMP_Handle)listHandle,&(node->elem));
            node = (ListMP_Node *) (sharedAddr + 192);
            node->id = 222;
            Osal_printf ("List insertElement ID = %d ",node->id);
            Osal_printf ("before element ID = %d\n",
                                           (((ListMP_Node *) (sharedAddr + 128))->id));
            status = ListMP_insert((ListMP_Handle)listHandle,
                                   &(node->elem),
                                   &(((ListMP_Node *) (sharedAddr + 128))->elem));
            if(status < 0) {
                Osal_printf ("List insertElement failed\n");
            }
            else {
                node = (ListMP_Node *) ListMP_next(listHandle,NULL);
                Osal_printf ("Testing ListMP_next with NULL elem ID = %d\n",
                                                                          node->id);
                node = (ListMP_Node *) ListMP_prev(listHandle,NULL);
                Osal_printf ("Testing ListMP_prev with NULL elem ID = %d\n",
                                                                          node->id);

                Osal_printf ("Removing element from list ID = %d\n",
                                         (((ListMP_Node *) (sharedAddr + 64))->id));
                status = ListMP_remove((ListMP_Handle)listHandle,
                                         &(((ListMP_Node *) (sharedAddr + 64))->elem));
                if(status < 0){
                    Osal_printf ("List removeElement failed\n");
                }

                node = (ListMP_Node *)ListMP_getTail((ListMP_Handle)listHandle);
                Osal_printf ("Testing ListMP_getTail ID = %d\n",node->id);

                node = (ListMP_Node *) (sharedAddr + 64);
                node->id = 555;
                Osal_printf ("Testing ListMP_putHead ID = %d\n",node->id);
                node = (ListMP_Node *)ListMP_putHead((ListMP_Handle)listHandle,
                                                     &(node->elem));

                node = (ListMP_Node *)ListMP_getHead(listHandle);
                Osal_printf ("Testing ListMP_getHead ID = %d\n",node->id);
            }

            Osal_printf ("Closing the shared List\n");
            status = ListMP_close(&listHandle);
            if(status < 0){
                Osal_printf ("ListMP_close failed\n");
            }

            Osal_printf("Closing the GatePeterson\n");
            status = GatePeterson_close(&ListMPApp_gateHandle);
            if(status < 0){
                Osal_printf ("GatePeterson_close failed\n");
            }
        }
    }

    Osal_printf ("Leaving ListMPApp_execute\n");
    return (0);
}


Int
ListMPApp_shutdown (Void)
{
    Int32 status = 0;

    Osal_printf ("\nEntered ListMPApp_shutdown\n");

#if defined(SYSLINK_USE_SYSMGR)
    SharedRegion_remove (0);
    ProcUtil_stop ();
    status = ProcMgr_close (&procHandle);
    Osal_printf ("ProcMgr_close status: [0x%x]\n", status);
    SysMgr_destroy ();
#else

    status = ListMPSharedMemory_destroy();
    Osal_printf ("ListMPSharedMemory_destroy status: [0x%x]\n", status);

    status = GatePeterson_destroy ();
    Osal_printf ("GatePeterson_destroy status: [0x%x]\n", status);

    SharedRegion_remove (0);

    status = SharedRegion_destroy();
    Osal_printf ("SharedRegion_destroy status: [0x%x]\n", status);

    ProcUtil_stop ();
    ProcUtil_shutdown ();
    UsrUtilsDrv_destroy ();
#endif

    Osal_printf ("Leaving ListMPApp_shutdown\n");
    return (0);
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
