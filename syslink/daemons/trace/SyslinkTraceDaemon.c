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
 *  @file   SyslinkTraceDaemon.c
 *
 *  @brief  Daemon for Syslink trace
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>
#include <UsrUtilsDrv.h>
#include <Memory.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* Use the Ducati HEAP3 for trace buffer
 * SYSM3 VA address would be 0x81FE0000, APPM3 is 0x81FF0000
 */
#define SYSM3_TRACE_BUFFER_PHYS_ADDR    0x9FFE0000
#define APPM3_TRACE_BUFFER_PHYS_ADDR    0X9FFF0000

#define TRACE_BUFFER_SIZE               0x10000

#define TIMEOUT_SECS                    1


// pull char from queue
void printSysM3Traces(void *arg)
{

    Int                             status = 0;
    Memory_MapInfo                  traceinfo;
    UInt32                          numOfBytesInBuffer = 0;
    UInt32  *readPointer;
    UInt32  *writePointer;
    char    *traceBuffer;

    UsrUtilsDrv_setup ();
    /* Get the user virtual address of the buffer */
    traceinfo.src  = SYSM3_TRACE_BUFFER_PHYS_ADDR;
    traceinfo.size = TRACE_BUFFER_SIZE;
    Osal_printf("\nSpawning SysM3 trace thread\n ");
    status = Memory_map (&traceinfo);
    readPointer = (UInt32 *)traceinfo.dst;
    writePointer = (UInt32 *)(traceinfo.dst + 0x4);
    traceBuffer = (char *)(traceinfo.dst + 0x8);

    /* Initialze read and write indexes to zero */
    *readPointer = 0;
    *writePointer = 0;
    do {

        do {
           sleep(TIMEOUT_SECS);
        } while (*readPointer == *writePointer);

        if ( *readPointer < *writePointer ) {
            numOfBytesInBuffer = *writePointer - *readPointer;
        } else {
            numOfBytesInBuffer = (TRACE_BUFFER_SIZE - *readPointer) + *writePointer;
        }
        Osal_printf("\n[SYSM3]: ");
        while ( numOfBytesInBuffer-- )
        {
            printf("%c", traceBuffer[*readPointer]);
            if (traceBuffer[*readPointer] == '\n')
                Osal_printf("[SYSM3]: ");

            if (*readPointer == (TRACE_BUFFER_SIZE - 1))
                *readPointer = 0;
            else
                (*readPointer)++;
        }

    } while(1);
    Osal_printf ("Leaving printSysM3Traces thread function \n");
    return;
}


// pull char from queue
void printAppM3Traces(void *arg)
{

    Int                             status = 0;
    Memory_MapInfo                  traceinfo;
    UInt32                          numOfBytesInBuffer = 0;
    UInt32  *readPointer;
    UInt32  *writePointer;
    char    *traceBuffer;

    UsrUtilsDrv_setup ();
    /* Get the user virtual address of the buffer */
    traceinfo.src  = APPM3_TRACE_BUFFER_PHYS_ADDR;
    traceinfo.size = TRACE_BUFFER_SIZE;
    Osal_printf("\nSpawning APP-M3 trace thread\n ");
    status = Memory_map (&traceinfo);
    readPointer = (UInt32 *)traceinfo.dst;
    writePointer = (UInt32 *)(traceinfo.dst + 0x4);
    traceBuffer = (char *)(traceinfo.dst + 0x8);

    /* Initialze read and write indexes to zero */
    *readPointer = 0;
    *writePointer = 0;
    do {

        do {
           sleep(TIMEOUT_SECS);
        } while (*readPointer == *writePointer);

        if ( *readPointer < *writePointer ) {
            numOfBytesInBuffer = *writePointer - *readPointer;
        } else {
            numOfBytesInBuffer = (TRACE_BUFFER_SIZE - *readPointer) + *writePointer;
        }
        Osal_printf("\n[APPM3]: ");
        while ( numOfBytesInBuffer-- )
        {
            printf("%c", traceBuffer[*readPointer]);
            if (traceBuffer[*readPointer] == '\n')
                Osal_printf("[APPM3]: ");

            if (*readPointer == (TRACE_BUFFER_SIZE - 1))
                *readPointer = 0;
            else
                (*readPointer)++;
        }

    } while(1);
    Osal_printf ("Leaving printAppM3Traces thread function \n");
    return;
}


Int main (Int argc, Char * argv [])
{
    pid_t child_pid, child_sid;
    Int status;
    pthread_t                       thread_sys; /* server thread object */
    pthread_t                       thread_app; /* server thread object */

    Osal_printf("Spawning Ducati Trace daemon...\n");

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
    {
        Osal_printf("setsid failed!\n");
        exit(EXIT_FAILURE);     /* Failure */
    }
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        Osal_printf("chdir failed!\n");
        exit(EXIT_FAILURE);     /* Failure */
    }
    pthread_create (&thread_sys, NULL, (Void *)&printSysM3Traces,
                    NULL);
    pthread_create (&thread_app, NULL, (Void *)&printAppM3Traces,
                    NULL);
    pthread_join(thread_sys, NULL);
    Osal_printf("sysm3 trace thread exited\n");
    pthread_join(thread_app, NULL);
    Osal_printf("appm3 trace thread exited\n");

    return 0;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */