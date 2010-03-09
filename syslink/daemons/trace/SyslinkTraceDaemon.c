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
#include <semaphore.h>

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

sem_t semPrint;    /* Semaphore to allow only one thread to print at once */

/* pull char from queue */
Void printSysM3Traces (Void *arg)
{
    Int             status              = 0;
    Memory_MapInfo  traceinfo;
    UInt32          numOfBytesInBuffer  = 0;
    UInt32        * readPointer;
    UInt32        * writePointer;
    Char          * traceBuffer;

    UsrUtilsDrv_setup ();

    Osal_printf ("\nSpawning SysM3 trace thread\n ");

    /* Get the user virtual address of the buffer */
    traceinfo.src  = SYSM3_TRACE_BUFFER_PHYS_ADDR;
    traceinfo.size = TRACE_BUFFER_SIZE;
    status = Memory_map (&traceinfo);
    readPointer = (UInt32 *)traceinfo.dst;
    writePointer = (UInt32 *)(traceinfo.dst + 0x4);
    traceBuffer = (Char *)(traceinfo.dst + 0x8);

    /* Initialze read indexes to zero */
    *readPointer = 0;
    *writePointer = 0;
    do {
        do {
           sleep (TIMEOUT_SECS);
        } while (*readPointer == *writePointer);

        sem_wait(&semPrint);    /* Acquire exclusive access to printing */
        if ( *readPointer < *writePointer ) {
            numOfBytesInBuffer = (*writePointer) - (*readPointer);
        } else {
            numOfBytesInBuffer = ((TRACE_BUFFER_SIZE - 8) - (*readPointer)) + (*writePointer);
        }

        Osal_printf ("\n[SYSM3]: ");
        while ( numOfBytesInBuffer-- ) {
            if ((*readPointer) == (TRACE_BUFFER_SIZE - 8)){
                (*readPointer) = 0;
            }

            Osal_printf ("%c", traceBuffer[*readPointer]);
            if (traceBuffer[*readPointer] == '\n') {
                Osal_printf ("[SYSM3]: ");
            }

            (*readPointer)++;
        }
        sem_post(&semPrint);    /* Release exclusive access to printing */

    } while(1);

    Osal_printf ("Leaving printSysM3Traces thread function \n");
    return;
}


/* pull char from queue */
Void printAppM3Traces (Void *arg)
{
    Int             status              = 0;
    Memory_MapInfo  traceinfo;
    UInt32          numOfBytesInBuffer  = 0;
    UInt32        * readPointer;
    UInt32        * writePointer;
    Char          * traceBuffer;

    UsrUtilsDrv_setup ();

    Osal_printf ("\nSpawning APP-M3 trace thread\n ");

    /* Get the user virtual address of the buffer */
    traceinfo.src  = APPM3_TRACE_BUFFER_PHYS_ADDR;
    traceinfo.size = TRACE_BUFFER_SIZE;
    status = Memory_map (&traceinfo);
    readPointer = (UInt32 *)traceinfo.dst;
    writePointer = (UInt32 *)(traceinfo.dst + 0x4);
    traceBuffer = (Char *)(traceinfo.dst + 0x8);

    /* Initialze read and write indexes to zero */
    *readPointer = 0;
    *writePointer = 0;
    do {
        do {
           sleep (TIMEOUT_SECS);
        } while (*readPointer == *writePointer);

        sem_wait(&semPrint);    /* Acquire exclusive access to printing */
        if ( *readPointer < *writePointer ) {
            numOfBytesInBuffer = *writePointer - *readPointer;
        } else {
            numOfBytesInBuffer = ((TRACE_BUFFER_SIZE - 8) - *readPointer) + *writePointer;
        }

        Osal_printf ("\n[APPM3]: ");
        while ( numOfBytesInBuffer-- ) {
            if (*readPointer >= (TRACE_BUFFER_SIZE - 8)){
                *readPointer = 0;
            }

            Osal_printf ("%c", traceBuffer[*readPointer]);
            if (traceBuffer[*readPointer] == '\n') {
                Osal_printf ("[APPM3]: ");
            }

            (*readPointer)++;
        }
        sem_post(&semPrint);    /* Release exclusive access to printing */

    } while(1);

    Osal_printf ("Leaving printAppM3Traces thread function \n");
    return;
}


Int main (Int argc, Char * argv [])
{
    pid_t       child_pid;
    pid_t       child_sid;
    pthread_t   thread_sys; /* server thread object */
    pthread_t   thread_app; /* server thread object */

    Osal_printf ("Spawning Ducati Trace daemon...\n");

    /* Fork off the parent process */
    child_pid = fork ();
    if (child_pid < 0) {
        Osal_printf ("Spawning Trace daemon failed!\n");
        exit (EXIT_FAILURE);     /* Failure */
    }

    /* If we got a good PID, then we can exit the parent process. */
    if (child_pid > 0) {
        exit (EXIT_SUCCESS);    /* Succeess */
    }

    /* Change file mode mask */
    umask (0);

    /* Create a new SID for the child process */
    child_sid = setsid ();
    if (child_sid < 0)
    {
        Osal_printf ("setsid failed!\n");
        exit (EXIT_FAILURE);     /* Failure */
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        Osal_printf ("chdir failed!\n");
        exit (EXIT_FAILURE);     /* Failure */
    }

    sem_init(&semPrint, 0, 1);

    pthread_create (&thread_sys, NULL, (Void *)&printSysM3Traces,
                    NULL);
    pthread_create (&thread_app, NULL, (Void *)&printAppM3Traces,
                    NULL);

    pthread_join (thread_sys, NULL);
    Osal_printf ("SysM3 trace thread exited\n");
    pthread_join (thread_app, NULL);
    Osal_printf ("AppM3 trace thread exited\n");

    sem_destroy(&semPrint);

    return 0;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */