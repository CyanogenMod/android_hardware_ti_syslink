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

/* RCM headers */
#include <RcmServer.h>



/*
 *  ======== MemMgrThreadFxn ========
 */
Void MemMgrThreadFxn();

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


Int main (Int argc, Char * argv [])
{
    pid_t child_pid, child_sid;

    Osal_printf("Spawning TILER server daemon...\n");

    /* Fork off the parent process */
    child_pid = fork();
    if (child_pid < 0) {
        Osal_printf("Spawn daemon failed!\n");
        exit(EXIT_FAILURE);     /* Failure */
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (child_pid > 0) {
        Osal_printf("Spawn daemon succeeded!\n");
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

    // Launch!
    MemMgrThreadFxn();

    return 0;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
