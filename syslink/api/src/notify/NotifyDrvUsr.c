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
 *  @file       NotifyDrvUsr.c
 *
 *  @brief      User-side OS-specific implementation of Notify driver for Linux
 *
 *  ============================================================================
 */


/* Linux specific header files */
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <NotifyDrvUsr.h>

/* Module headers */
#include <NotifyDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for Notify.
 */
#define NOTIFY_DRIVER_NAME         "/dev/ipcnotify"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for Notify in this process.
 */
static Int32 NotifyDrvUsr_handle = -1;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 NotifyDrvUsr_refCount = 0;

/*!
 *  @brief  Thread handler for event receiver thread.
 */
static pthread_t  NotifyDrv_workerThread;


/** ============================================================================
 *  Forward declaration of internal functions
 *  ============================================================================
 */
/*!
 *  @brief      This is the worker thread for polling on events.
 *
 *  @param      arg module attributes
 *
 *  @sa
 */
Void _NotifyDrvUsr_eventWorker (Void * arg);


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the Notify driver.
 *
 *  @param  createThread  Flag to indicate whether to create thread or not.
 *
 *  @sa     NotifyDrvUsr_close
 */
Int
NotifyDrvUsr_open (Bool createThread)
{
    Int    status   = NOTIFY_SUCCESS;
    int    osStatus = 0;
    UInt32 pid;

    GT_1trace (curTrace, GT_ENTER, "NotifyDrvUsr_open", createThread);

    if (NotifyDrvUsr_refCount == 0) {
        /* TBD: Protection for refCount. */
        NotifyDrvUsr_refCount++;

        NotifyDrvUsr_handle = open (NOTIFY_DRIVER_NAME, O_SYNC | O_RDWR);
        if (NotifyDrvUsr_handle < 0) {
            perror ("Notify driver open: " NOTIFY_DRIVER_NAME);
            /*! @retval NOTIFY_E_OSFAILURE Failed to open Notify driver with
                        OS */
            status = NOTIFY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDrvUsr_open",
                                 status,
                                 "Failed to open Notify driver with OS!");
        }
        else {
            osStatus = fcntl (NotifyDrvUsr_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval NOTIFY_E_OSFAILURE Failed to set file descriptor
                                                flags */
                status = NOTIFY_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NotifyDrvUsr_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else {
                if (createThread == TRUE) {
                    pid = getpid ();
                    status = NotifyDrvUsr_ioctl (CMD_NOTIFY_ATTACH, &pid);
                    if (status < 0) {
                        GT_setFailureReason (curTrace,
                                             GT_4CLASS,
                                             "NotifyDrvUsr_close",
                                             status,
                                             "Notify attach failed on kernel "
                                             "side!");
                    }
                    else {
                        /* Create the pthread */
                        pthread_create (&NotifyDrv_workerThread,
                                        NULL,
                                        (Ptr) _NotifyDrvUsr_eventWorker,
                                        NULL);
                        if (NotifyDrv_workerThread == (UInt32) NULL) {
                            /*! @retval NOTIFY_E_OSFAILURE Failed to create
                                                           Notify thread */
                            status = NOTIFY_E_OSFAILURE;
                            GT_setFailureReason (curTrace,
                                                 GT_4CLASS,
                                                 "NotifyDrvUsr_open",
                                                 status,
                                                 "Failed to create Notify "
                                                 "thread!");
                        }
                    }
                }
            }
        }
    }
    else {
        /* TBD: Protection for refCount. */
        NotifyDrvUsr_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDrvUsr_open", status);

    /*! @retval NOTIFY_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the Notify driver.
 *
 *  @param  deleteThread  Flag to indicate whether to delete thread or not.
 *
 *  @sa     NotifyDrvUsr_open
 */
Int
NotifyDrvUsr_close (Bool deleteThread)
{
    Int    status      = NOTIFY_SUCCESS;
    int    osStatus    = 0;
    UInt32 pid;

    GT_1trace (curTrace, GT_ENTER, "NotifyDrvUsr_close", deleteThread);

    /* TBD: Protection for refCount. */
    if (NotifyDrvUsr_refCount == 1) {
        if (deleteThread == TRUE) {
            pid = getpid ();
            status = NotifyDrvUsr_ioctl (CMD_NOTIFY_DETACH, &pid);
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NotifyDrvUsr_close",
                                     status,
                                     "Notify detach failed on kernel side!");
            }

            pthread_join (NotifyDrv_workerThread, NULL);
        }
        NotifyDrvUsr_refCount--;

        osStatus = close (NotifyDrvUsr_handle);
        if (osStatus != 0) {
            perror ("Notify driver close: " NOTIFY_DRIVER_NAME);
            /*! @retval NOTIFY_E_OSFAILURE Failed to open Notify driver with
                        OS */
            status = NOTIFY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDrvUsr_close",
                                 status,
                                 "Failed to close Notify driver with OS!");
        }
        else {
            NotifyDrvUsr_handle = -1;
        }
    }
    else {
        NotifyDrvUsr_refCount--;
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDrvUsr_close", status);

    /*! @retval NOTIFY_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to invoke the APIs through ioctl.
 *
 *  @param  cmd     Command for driver ioctl
 *  @param  args    Arguments for the ioctl command
 *
 *  @sa
 */
Int
NotifyDrvUsr_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = NOTIFY_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "NotifyDrvUsr_ioctl", cmd, args);

    GT_assert (curTrace, (NotifyDrvUsr_refCount > 0));

    osStatus = ioctl (NotifyDrvUsr_handle, cmd, args);
    if (osStatus < 0) {
        /*! @retval NOTIFY_E_OSFAILURE Driver ioctl failed */
        status = NOTIFY_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDrvUsr_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((Notify_CmdArgs *) args)->apiStatus;
    }
    status = ((Notify_CmdArgs *) args)->apiStatus;
    GT_1trace (curTrace, GT_LEAVE, "NotifyDrvUsr_ioctl", status);

    /*! @retval NOTIFY_SUCCESS Operation successfully completed. */
    return status;
}


/** ============================================================================
 *  Internal functions
 *  ============================================================================
 */
/*!
 *  @brief      This is the worker thread which polls for events.
 *
 *  @param      attrs module attributes
 *
 *  @sa
 */
Void
_NotifyDrvUsr_eventWorker (Void * arg)
{
    Int32                 status = NOTIFY_SUCCESS;
    UInt32                nRead  = 0;
    NotifyDrv_EventPacket packet;
    sigset_t              blockSet;

    GT_1trace (curTrace, GT_ENTER, "_NotifyDrvUsr_eventWorker", arg);

    if (sigfillset (&blockSet) != 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "_NotifyDrvUsr_eventWorker",
                             NOTIFY_E_OSFAILURE,
                             "Event worker thread error in sigfillset!");
        return;
        
    }

    if (pthread_sigmask (SIG_BLOCK, &blockSet, NULL) != 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "_NotifyDrvUsr_eventWorker",
                             NOTIFY_E_OSFAILURE,
                             "Event worker thread error in setting sigmask!");
        return;
    }

    while (status >= 0) {
        memset (&packet, 0, sizeof (NotifyDrv_EventPacket));
        packet.pid = getpid ();
        nRead = read (NotifyDrvUsr_handle,
                      &packet,
                      sizeof (NotifyDrv_EventPacket));
        if (0 == nRead) {
            /* check for termination packet */
            if (packet.isExit  == TRUE) {
                return;
            }

            if (packet.func != NULL) {
                packet.func (packet.procId,
                             packet.eventNo,
                             packet.param,
                             packet.data);
            }
        }
    }

    GT_0trace (curTrace, GT_LEAVE, "_NotifyDrvUsr_eventWorker");
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
