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

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* IPC headers */
#include <IpcUsr.h>
#include <ProcMgr.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/SharedRegion.h>

/* Sample headers */
#include <MemMgrServer_config.h>


/*
 *  ======== MemMgrThreadFxn ========
 */
Void MemMgrThreadFxn();

ProcMgr_Handle                  procMgrHandleSysM3;
ProcMgr_Handle                  procMgrHandleAppM3;
Bool                            appM3Client          = FALSE;
UInt16                          remoteIdSysM3;
UInt16                          remoteIdAppM3;
extern sem_t                    semDaemonWait;

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

    status = ProcMgr_detach (procMgrHandleAppM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_detach(AppM3): status = 0x%x\n", status);
    }

    status = ProcMgr_close (&procMgrHandleAppM3);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_close(AppM3): status = 0x%x\n", status);
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
#if 0
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
#endif
    UInt16                          procId;
#if defined (SYSLINK_USE_LOADER)
    UInt32                          fileId;
#endif
    Int                             status = 0;
    ProcMgr_AttachParams            attachParams;
    ProcMgr_State                   state;

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
