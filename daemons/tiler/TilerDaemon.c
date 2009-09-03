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
 *  @file   TilerDaemon.c
 *
 *  @brief  Daemon implementation of TILER server
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* RCM headers */
#include <RcmServer.h>



/*
 *  ======== TilerServerThreadFxn ========
 */
int TilerServerThreadFxn();

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
        exit(1);     /* Failure */
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (child_pid > 0) {
        Osal_printf("Spawn daemon succeeded!\n");
        exit(0);    /* Succeess */
    }

    /* Create a new SID for the child process */
    child_sid = setsid();
    if (child_sid < 0)
        exit(0);

    /* Close standard file descriptors */
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    // Launch!
    TilerServerThreadFxn();

    return 0;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
