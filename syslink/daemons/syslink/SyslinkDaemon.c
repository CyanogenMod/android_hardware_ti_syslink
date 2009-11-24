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
#include <sys/types.h>
#include <sys/stat.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* IPC headers */
#include <SysMgr.h>
#include <ProcMgr.h>

/* Sample headers */
#include <MemMgrServer_config.h>


/*
 *  ======== MemMgrThreadFxn ========
 */
Void MemMgrThreadFxn();


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*
 *  ======== ipc_setup ========
 */
Int ipc_setup(Char * sysM3ImageName, Char * appM3ImageName)
{
    SysMgr_Config                   config;
    ProcMgr_StopParams              stopParams;
    ProcMgr_StartParams             start_params;
    ProcMgr_Handle                  procMgrHandle_server;
    UInt32                          entry_point = 0;
    UInt16                          remoteIdSysM3;
    UInt16                          remoteIdAppM3;
    UInt32                          shAddrBase;
    UInt32                          shAddrBase1;
    UInt16                          procId;
#if defined (SYSLINK_USE_LOADER)
    UInt32                          fileId;
#endif

    Int                             status = 0;
    Bool                            appM3Client = FALSE;

    if(appM3ImageName != NULL)
        appM3Client = TRUE;
    else
        appM3Client = FALSE;

    SysMgr_getConfig (&config);
    status = SysMgr_setup (&config);
    if (status < 0) {
        Osal_printf ("Error in SysMgr_setup [0x%x]\n", status);
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
    status = ProcMgr_open (&procMgrHandle_server,
                           procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        goto exit_sysmgr_destroy;
    }
    else {
        Osal_printf ("ProcMgr_open status [0x%x]\n", status);
        /* Get the address of the shared region in kernel space. */
        status = ProcMgr_translateAddr (procMgrHandle_server,
                                        (Ptr) &shAddrBase,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) SHAREDMEM,
                                        ProcMgr_AddrType_SlaveVirt);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                         status);
            goto exit_procmgr_close;
        }
        else {
            Osal_printf ("Virt address of shared address base #1:"
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
                Osal_printf ("Error in ProcMgr_translateAddr [0x%x]\n",
                             status);
                goto exit_procmgr_close;
            }
            else {
                Osal_printf ("Virt address of shared address base #2:"
                             " [0x%x]\n", shAddrBase1);
            }
        }
    }
    if (status >= 0) {
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (SHAREDMEMNUMBER,
                                   (Ptr) shAddrBase,
                                   SHAREDMEMSIZE);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add [0x%x]\n", status);
            goto exit_procmgr_close;
        }
    }

    if (status >= 0) {
        /* Add the region to SharedRegion module. */
        status = SharedRegion_add (SHAREDMEMNUMBER1,
                                   (Ptr) shAddrBase1,
                                   SHAREDMEMSIZE1);
        if (status < 0) {
            Osal_printf ("Error in SharedRegion_add1 [0x%x]\n", status);
            goto exit_procmgr_close;
        }
    }


#if defined(SYSLINK_USE_LOADER)
    Osal_printf ("SYSM3 Load: loading the SYSM3 image %s\n",
                sysM3ImageName);

    status = ProcMgr_load (procMgrHandle_server, sysM3ImageName, 2,
                            &sysM3ImageName, &entry_point, &fileId,
                            remoteIdSysM3);
    if(status < 0) {
        Osal_printf ("Error in ProcMgr_load, status [0x%x]\n", status);
        goto exit_procmgr_close;
    }

#endif
    start_params.proc_id = remoteIdSysM3;
    Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
    status  = ProcMgr_start(procMgrHandle_server, entry_point, &start_params);
    if(status < 0) {
        Osal_printf ("Error in ProcMgr_start, status [0x%x]\n", status);
        goto exit_procmgr_close;
    }

    if(appM3Client) {
#if defined(SYSLINK_USE_LOADER)
        Osal_printf ("APPM3 Load: loading the APPM3 image %s\n",
                    appM3ImageName);
        status = ProcMgr_load (procMgrHandle_server, appM3ImageName, 2,
                              &appM3ImageName, &entry_point, &fileId,
                              remoteIdAppM3);
        if(status < 0) {
            Osal_printf ("Error in ProcMgr_load, status [0x%x]\n", status);
            goto exit_procmgr_stop_sysm3;
        }
#endif
        start_params.proc_id = remoteIdAppM3;
        Osal_printf("Starting ProcMgr for procID = %d\n", start_params.proc_id);
        status  = ProcMgr_start(procMgrHandle_server, entry_point,
                                &start_params);
        if(status < 0) {
            Osal_printf ("Error in ProcMgr_start, status [0x%x]\n", status);
            goto exit_procmgr_stop_sysm3;
        }
    }

    Osal_printf("IPC setup completed successfully!\n");
    return 0;

exit_procmgr_stop_appm3:
    if(appM3Client) {
        stopParams.proc_id = remoteIdAppM3;
        status = ProcMgr_stop(procMgrHandle_server, &stopParams);
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_stop(%d): status = 0x%x\n",
                stopParams.proc_id, status);
        }
    }

exit_procmgr_stop_sysm3:
    stopParams.proc_id = remoteIdSysM3;
    status = ProcMgr_stop(procMgrHandle_server, &stopParams);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_stop(%d): status = 0x%x\n",
            stopParams.proc_id, status);
    }

exit_procmgr_close:
    status = ProcMgr_close(&procMgrHandle_server);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_close: status = 0x%x\n", status);
    }

exit_sysmgr_destroy:
    status = SysMgr_destroy();
    if (status < 0) {
        Osal_printf ("Error in SysMgr_destroy: status = 0x%x\n", status);
    }
exit:
    return (-1);
}


Int main (Int argc, Char * argv [])
{
    pid_t child_pid, child_sid;
    Int status;
    FILE *fp;
    Bool calledIpcSetup = false;

    Osal_printf("Spawning TILER server daemon...\n");

    /* Fork off the parent process */
    child_pid = fork();
    if (child_pid < 0) {
        Osal_printf("Spawn daemon failed!\n");
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
        Osal_printf("Invalid arguments to Daemon.  Usage:\n");
        Osal_printf("\tRunning SysM3 only:\n\t\t./syslink_daemon.out <SysM3 image file>\n");
        Osal_printf("\tRunning SysM3 and AppM4:\n\t\t./syslink_daemon.out <SysM3 image file> <AppM3 image file>\n");
        Osal_printf("\t(full paths must be provided for image files)\n");
        break;
    case 2:     // load SysM3 only
        // Test for file's presence
        fp = fopen(argv[1], "rb");
        if(fp != NULL) {
            fclose(fp);
            status = ipc_setup(argv[1], NULL);
            calledIpcSetup = true;
        }
        else
            Osal_printf("File %s could not be opened.\n", argv[1]);
        break;
    case 3:     // load AppM3 and SysM3
    default:
        // Test for file's presence
        fp = fopen(argv[1], "rb");
        if(fp != NULL) {
            fclose(fp);
            fp = fopen(argv[2], "rb");
            if(fp != NULL) {
                fclose(fp);
                status = ipc_setup(argv[1], argv[2]);
                calledIpcSetup = true;
            }
            else
                Osal_printf("File %s could not be opened.\n", argv[2]);
        }
        else
            Osal_printf("File %s could not be opened.\n", argv[1]);
        break;
    }
    if(calledIpcSetup) {
        if(status < 0) {
            Osal_printf("ipc_setup failed!\n");
            return (-1);        // Quit if there was a setup error
        } else {
            Osal_printf("ipc_setup succeeded!\n");
            MemMgrThreadFxn();
        }
    }

    return 0;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
