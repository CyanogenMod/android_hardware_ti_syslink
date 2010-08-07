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
/*==============================================================================
 *  @file   SyslinkDaemon.c
 *
 *  @brief  Daemon for Syslink functions
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
/* OSAL & Utils headers */
#include <OsalPrint.h>
#include <Memory.h>

/* IPC headers */
#include <IpcUsr.h>
#include <ProcMgr.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MessageQ.h>

/* Sample headers */
#include <MemMgrServer_config.h>

/* Defines for the default HeapBufMP being configured in the System */
#define RCM_MSGQ_TILER_HEAPNAME         "Heap0"
#define RCM_MSGQ_TILER_HEAP_BLOCKS      256
#define RCM_MSGQ_TILER_HEAP_ALIGN       128
#define RCM_MSGQ_TILER_MSGSIZE          256
#define RCM_MSGQ_TILER_HEAPID           0
#define RCM_MSGQ_DOMX_HEAPNAME          "Heap1"
#define RCM_MSGQ_DOMX_HEAP_BLOCKS       256
#define RCM_MSGQ_DOMX_HEAP_ALIGN        128
#define RCM_MSGQ_DOMX_MSGSIZE           256
#define RCM_MSGQ_DOMX_HEAPID            1
#define RCM_MSGQ_HEAP_SR                1


/*
 *  ======== MemMgrThreadFxn ========
 */
Void MemMgrThreadFxn();

ProcMgr_Handle                  procMgrHandleSysM3;
ProcMgr_Handle                  procMgrHandleAppM3;
Bool                            appM3Client         = FALSE;
UInt16                          remoteIdSysM3;
UInt16                          remoteIdAppM3;
extern sem_t                    semDaemonWait;
HeapBufMP_Handle                heapHandle          = NULL;
SizeT                           heapSize            = 0;
Ptr                             heapBufPtr          = NULL;
HeapBufMP_Handle                heapHandle1         = NULL;
SizeT                           heapSize1           = 0;
Ptr                             heapBufPtr1         = NULL;
IHeap_Handle                    srHeap              = NULL;
pthread_t                       mmu_fault_handle = NULL;

static void mmu_fault_handler(void)
{
    int efd;
    int status;
    int ret;

    status = ProcMgr_waitForEvent(PROC_SYSM3, PROC_MMU_FAULT, -1);

    /* Initiate cleanup */
    sem_post(&semDaemonWait);
}

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*
 *  ======== signal_handler ========
 */
static Void signal_handler(Int sig)
{
    Osal_printf ("\nexiting from the syslink daemon\n ");
    sem_post(&semDaemonWait);
}


/*
 *  ======== ipc_cleanup ========
 */
Void ipcCleanup()
{
    ProcMgr_StopParams stopParams;
    Int                status = 0;

    /* Cleanup the default HeapBufMP registered with MessageQ */
    status = MessageQ_unregisterHeap (RCM_MSGQ_DOMX_HEAPID);
    if (status < 0) {
        Osal_printf ("Error in MessageQ_unregisterHeap [0x%x]\n", status);
    }

    if (heapHandle1) {
        status = HeapBufMP_delete (&heapHandle1);
        if (status < 0) {
            Osal_printf ("Error in HeapBufMP_delete [0x%x]\n", status);
        }
    }

    if (heapBufPtr1) {
        Memory_free (srHeap, heapBufPtr1, heapSize1);
    }

    status = MessageQ_unregisterHeap (RCM_MSGQ_TILER_HEAPID);
    if (status < 0) {
        Osal_printf ("Error in MessageQ_unregisterHeap [0x%x]\n", status);
    }

    if (heapHandle) {
        status = HeapBufMP_delete (&heapHandle);
        if (status < 0) {
            Osal_printf ("Error in HeapBufMP_delete [0x%x]\n", status);
        }
    }

    if (heapBufPtr) {
        Memory_free (srHeap, heapBufPtr, heapSize);
    }

    if(appM3Client) {
        stopParams.proc_id = remoteIdAppM3;
        status = ProcMgr_stop (procMgrHandleAppM3, &stopParams);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_stop(%d): status = 0x%x\n",
                            stopParams.proc_id, status);
        }
    }

    stopParams.proc_id = remoteIdSysM3;
    status = ProcMgr_stop (procMgrHandleSysM3, &stopParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_stop(%d): status = 0x%x\n",
                        stopParams.proc_id, status);
    }

    if(appM3Client) {
        status = ProcMgr_detach (procMgrHandleAppM3);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_detach(AppM3): status = 0x%x\n", status);
        }

        status = ProcMgr_close (&procMgrHandleAppM3);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_close(AppM3): status = 0x%x\n", status);
        }
    }

    status = ProcMgr_detach (procMgrHandleSysM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_detach(SysM3): status = 0x%x\n", status);
    }

    status = ProcMgr_close (&procMgrHandleSysM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_close(SysM3): status = 0x%x\n", status);
    }

    status = Ipc_destroy ();
    if (status < 0) {
        Osal_printf ("Error in Ipc_destroy: status = 0x%x\n", status);
    }

    Osal_printf ("Done cleaning up ipc!\n");
}


/*
 *  ======== ipcSetup ========
 */
Int ipcSetup (Char * sysM3ImageName, Char * appM3ImageName)
{
    Ipc_Config                      config;
    ProcMgr_StopParams              stopParams;
    ProcMgr_StartParams             startParams;
    UInt32                          entryPoint = 0;
    UInt16                          procId;
#if defined (SYSLINK_USE_LOADER)
    UInt32                          fileId;
#endif
    Int                             status = 0;
    ProcMgr_AttachParams            attachParams;
    ProcMgr_State                   state;
    HeapBufMP_Params                heapbufmpParams;
    Int                             i;
    UInt32                          srCount;
    SharedRegion_Entry              srEntry;

    if(appM3ImageName != NULL)
        appM3Client = TRUE;
    else
        appM3Client = FALSE;

    Ipc_getConfig (&config);
    status = Ipc_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in Ipc_setup [0x%x]\n", status);
        goto exit;
    }

    /* Get MultiProc IDs by name. */
    remoteIdSysM3 = MultiProc_getId (SYSM3_PROC_NAME);
    Osal_printf ("MultiProc_getId remoteId: [0x%x]\n", remoteIdSysM3);
    remoteIdAppM3 = MultiProc_getId (APPM3_PROC_NAME);
    Osal_printf ("MultiProc_getId remoteId: [0x%x]\n", remoteIdAppM3);
    procId = remoteIdSysM3;
    Osal_printf ("MultiProc_getId procId: [0x%x]\n", procId);

    printf("RCM procId= %d\n", procId);
    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandleSysM3, procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        goto exit_ipc_destroy;
    }
    else {
        Osal_printf ("ProcMgr_open Status [0x%x]\n", status);
        ProcMgr_getAttachParams (NULL, &attachParams);
        /* Default params will be used if NULL is passed. */
        status = ProcMgr_attach (procMgrHandleSysM3, &attachParams);
        if (status < 0) {
            Osal_printf ("ProcMgr_attach failed [0x%x]\n", status);
        }
        else {
            Osal_printf ("ProcMgr_attach status: [0x%x]\n", status);
            state = ProcMgr_getState (procMgrHandleSysM3);
            Osal_printf ("After attach: ProcMgr_getState\n"
                         "    state [0x%x]\n", status);
        }
    }

    if (status >= 0 && appM3Client) {
        procId = remoteIdAppM3;
        Osal_printf ("MultiProc_getId procId: [0x%x]\n", procId);

        /* Open a handle to the ProcMgr instance. */
        status = ProcMgr_open (&procMgrHandleAppM3, procId);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
            goto exit_ipc_destroy;
        }
        else {
            Osal_printf ("ProcMgr_open Status [0x%x]\n", status);
            ProcMgr_getAttachParams (NULL, &attachParams);
            /* Default params will be used if NULL is passed. */
            status = ProcMgr_attach (procMgrHandleAppM3, &attachParams);
            if (status < 0) {
                Osal_printf ("ProcMgr_attach failed [0x%x]\n", status);
            }
            else {
                Osal_printf ("ProcMgr_attach status: [0x%x]\n", status);
                state = ProcMgr_getState (procMgrHandleAppM3);
                Osal_printf ("After attach: ProcMgr_getState\n"
                             "    state [0x%x]\n", status);
            }
        }
    }

#if defined(SYSLINK_USE_LOADER)
    Osal_printf ("SYSM3 Load: loading the SYSM3 image %s\n",
                sysM3ImageName);

    status = ProcMgr_load (procMgrHandleSysM3, sysM3ImageName, 2,
                            &sysM3ImageName, &entryPoint, &fileId,
                            remoteIdSysM3);
    if(status < 0) {
        Osal_printf ("Error in ProcMgr_load, status [0x%x]\n", status);
        goto exit_procmgr_close_sysm3;
    }
#endif
    startParams.proc_id = remoteIdSysM3;
    Osal_printf ("Starting ProcMgr for procID = %d\n", startParams.proc_id);
    status  = ProcMgr_start(procMgrHandleSysM3, entryPoint, &startParams);
    if(status < 0) {
        Osal_printf ("Error in ProcMgr_start, status [0x%x]\n", status);
        goto exit_procmgr_close_sysm3;
    }

    if(appM3Client) {
#if defined(SYSLINK_USE_LOADER)
        Osal_printf ("APPM3 Load: loading the APPM3 image %s\n",
                    appM3ImageName);
        status = ProcMgr_load (procMgrHandleAppM3, appM3ImageName, 2,
                              &appM3ImageName, &entryPoint, &fileId,
                              remoteIdAppM3);
        if(status < 0) {
            Osal_printf ("Error in ProcMgr_load, status [0x%x]\n", status);
            goto exit_procmgr_stop_sysm3;
        }
#endif
        startParams.proc_id = remoteIdAppM3;
        Osal_printf ("Starting ProcMgr for procID = %d\n", startParams.proc_id);
        status  = ProcMgr_start(procMgrHandleAppM3, entryPoint,
                                &startParams);
        if(status < 0) {
            Osal_printf ("Error in ProcMgr_start, status [0x%x]\n", status);
            goto exit_procmgr_stop_sysm3;
        }
    }

    srCount = SharedRegion_getNumRegions();
    Osal_printf ("SharedRegion_getNumRegions = %d\n", srCount);
    for (i = 0; i < srCount; i++) {
        status = SharedRegion_getEntry (i, &srEntry);
        Osal_printf ("SharedRegion_entry #%d: base = 0x%x len = 0x%x "
                        "ownerProcId = %d isValid = %d cacheEnable = %d "
                        "cacheLineSize = 0x%x createHeap = %d name = %s\n",
                        i, srEntry.base, srEntry.len, srEntry.ownerProcId,
                        (Int)srEntry.isValid, (Int)srEntry.cacheEnable,
                        srEntry.cacheLineSize, (Int)srEntry.createHeap,
                        srEntry.name);
    }

    /* Create the heap to be used by RCM and register it with MessageQ */
    /* TODO: Do this dynamically by reading from the IPC config from the
     *       baseimage using Ipc_readConfig() */
    if (status >= 0) {
        HeapBufMP_Params_init (&heapbufmpParams);
        heapbufmpParams.sharedAddr = NULL;
        heapbufmpParams.align      = RCM_MSGQ_TILER_HEAP_ALIGN;
        heapbufmpParams.numBlocks  = RCM_MSGQ_TILER_HEAP_BLOCKS;
        heapbufmpParams.blockSize  = RCM_MSGQ_TILER_MSGSIZE;
        heapSize = HeapBufMP_sharedMemReq (&heapbufmpParams);
        Osal_printf ("heapSize = 0x%x\n", heapSize);

        srHeap = SharedRegion_getHeap (RCM_MSGQ_HEAP_SR);
        if (srHeap == NULL) {
            status = MEMORYOS_E_FAIL;
            Osal_printf ("SharedRegion_getHeap failed for srHeap:"
                         " [0x%x]\n", srHeap);
            goto exit_procmgr_stop_sysm3;
        }
        else {
            Osal_printf ("Before Memory_alloc = 0x%x\n", srHeap);
            heapBufPtr = Memory_alloc (srHeap, heapSize, 0);
            if (heapBufPtr == NULL) {
                status = MEMORYOS_E_MEMORY;
                Osal_printf ("Memory_alloc failed for ptr: [0x%x]\n",
                             heapBufPtr);
                goto exit_procmgr_stop_sysm3;
            }
            else {
                heapbufmpParams.name           = RCM_MSGQ_TILER_HEAPNAME;
                heapbufmpParams.sharedAddr     = heapBufPtr;
                Osal_printf ("Before HeapBufMP_Create: [0x%x]\n", heapBufPtr);
                heapHandle = HeapBufMP_create (&heapbufmpParams);
                if (heapHandle == NULL) {
                    status = HeapBufMP_E_FAIL;
                    Osal_printf ("HeapBufMP_create failed for Handle:"
                                 "[0x%x]\n", heapHandle);
                    goto exit_procmgr_stop_sysm3;
                }
                else {
                    /* Register this heap with MessageQ */
                    status = MessageQ_registerHeap (heapHandle,
                                                    RCM_MSGQ_TILER_HEAPID);
                    if (status < 0) {
                        Osal_printf ("MessageQ_registerHeap failed!\n");
                        goto exit_procmgr_stop_sysm3;
                    }
                }
            }
        }
    }

    if (status >= 0) {
        HeapBufMP_Params_init (&heapbufmpParams);
        heapbufmpParams.sharedAddr = NULL;
        heapbufmpParams.align      = RCM_MSGQ_DOMX_HEAP_ALIGN;
        heapbufmpParams.numBlocks  = RCM_MSGQ_DOMX_HEAP_BLOCKS;
        heapbufmpParams.blockSize  = RCM_MSGQ_DOMX_MSGSIZE;
        heapSize1 = HeapBufMP_sharedMemReq (&heapbufmpParams);
        Osal_printf ("heapSize1 = 0x%x\n", heapSize1);

        heapBufPtr1 = Memory_alloc (srHeap, heapSize1, 0);
        if (heapBufPtr1 == NULL) {
            status = MEMORYOS_E_MEMORY;
            Osal_printf ("Memory_alloc failed for ptr: [0x%x]\n",
                         heapBufPtr1);
            goto exit_procmgr_stop_sysm3;
        }
        else {
            heapbufmpParams.name           = RCM_MSGQ_DOMX_HEAPNAME;
            heapbufmpParams.sharedAddr     = heapBufPtr1;
            Osal_printf ("Before HeapBufMP_Create: [0x%x]\n", heapBufPtr1);
            heapHandle1 = HeapBufMP_create (&heapbufmpParams);
            if (heapHandle1 == NULL) {
                status = HeapBufMP_E_FAIL;
                Osal_printf ("HeapBufMP_create failed for Handle:"
                             "[0x%x]\n", heapHandle1);
                goto exit_procmgr_stop_sysm3;
            }
            else {
                /* Register this heap with MessageQ */
                status = MessageQ_registerHeap (heapHandle1,
                                                RCM_MSGQ_DOMX_HEAPID);
                if (status < 0) {
                    Osal_printf ("MessageQ_registerHeap failed!\n");
                    goto exit_procmgr_stop_sysm3;
                }
            }
        }
    }

    Osal_printf ("=== SysLink-IPC setup completed successfully!===\n");
    return 0;

exit_procmgr_stop_sysm3:
    stopParams.proc_id = remoteIdSysM3;
    status = ProcMgr_stop (procMgrHandleSysM3, &stopParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_stop(%d): status = 0x%x\n",
            stopParams.proc_id, status);
    }

exit_procmgr_close_sysm3:
    status = ProcMgr_close (&procMgrHandleSysM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_close: status = 0x%x\n", status);
    }

exit_ipc_destroy:
    status = Ipc_destroy ();
    if (status < 0) {
        Osal_printf ("Error in Ipc_destroy: status = 0x%x\n", status);
    }

exit:
    return (-1);
}


Int main (Int argc, Char * argv [])
{
    pid_t   child_pid;
    pid_t   child_sid;
    Int     status;
    FILE  * fp;
    Bool    calledIpcSetup = false;

    Osal_printf ("Spawning TILER server daemon...\n");

    /* Fork off the parent process */
    child_pid = fork();
    if (child_pid < 0) {
        Osal_printf ("Spawn daemon failed!\n");
        exit(EXIT_FAILURE);     /* Failure */
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (child_pid > 0) {
        exit(EXIT_SUCCESS);    /* Succeess */
    }

    /* Change file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    child_sid = setsid();
    if (child_sid < 0)
        exit(EXIT_FAILURE);     /* Failure */

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);     /* Failure */
    }

    /* Close standard file descriptors */
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    // Determine args
    switch(argc) {
    case 0:
    case 1:
        status = -1;
        Osal_printf ("Invalid arguments to Daemon.  Usage:\n");
        Osal_printf ("\tRunning SysM3 only:\n"
                     "\t\t./syslink_daemon.out <SysM3 image file>\n");
        Osal_printf ("\tRunning SysM3 and AppM3:\n"
                     "\t\t./syslink_daemon.out <SysM3 image file> "
                     "<AppM3 image file>\n");
        Osal_printf ("\t(full paths must be provided for image files)\n");
        break;
    case 2:     // load SysM3 only
        // Test for file's presence
        if (strlen (argv[1]) >= 1024) {
            Osal_printf ("Filename is too big\n");
            exit(EXIT_FAILURE);
        }
        fp = fopen(argv[1], "rb");
        if (fp != NULL) {
            fclose(fp);
            status = ipcSetup (argv[1], NULL);
            calledIpcSetup = true;
        }
        else
            Osal_printf ("File %s could not be opened.\n", argv[1]);
        break;
    case 3:     // load AppM3 and SysM3
    default:
        // Test for file's presence
        if ((strlen (argv[1]) >= 1024) || (strlen (argv[2]) >= 1024)){
            Osal_printf ("Filenames are too big\n");
            exit(EXIT_FAILURE);
        }
        fp = fopen(argv[1], "rb");
        if(fp != NULL) {
            fclose(fp);
            fp = fopen(argv[2], "rb");
            if(fp != NULL) {
                fclose(fp);
                status = ipcSetup(argv[1], argv[2]);
                calledIpcSetup = true;
            }
            else
                Osal_printf ("File %s could not be opened.\n", argv[2]);
        }
        else
            Osal_printf ("File %s could not be opened.\n", argv[1]);
        break;
    }
    if(calledIpcSetup) {
        if(status < 0) {
            Osal_printf ("ipcSetup failed!\n");
            return (-1);        // Quit if there was a setup error
        } else {
            Osal_printf ("ipcSetup succeeded!\n");

            /* Setup the signal handlers*/
            signal (SIGINT, signal_handler);
            signal (SIGKILL, signal_handler);
            signal (SIGTERM, signal_handler);

            /* Create the MMU fault handler thread */
            Osal_printf ("Create MMU fault handler thread.\n");
            pthread_create (&mmu_fault_handle, NULL,
                                (Void *)&mmu_fault_handler, NULL);


            MemMgrThreadFxn();

            /* IPC_Cleanup function*/
            ipcCleanup();
        }
    }

    return 0;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
