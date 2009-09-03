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
/*!
 *  @file       NameServerRemoteNotifyDrv.c
 *
 *  @brief      OS-specific implementation of NameServerRemoteNotify
 *              driver for Linux
 *
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <NameServerRemoteNotify.h>
#include <NameServerRemoteNotifyDrvDefs.h>

/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <IPCManager.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for NameServerRemoteNotify.
 */
#define NAMESERVERREMOTENOTIFY_DRIVER_NAME          \
                                     "/dev/syslinkipc/NameServerRemoteNotify"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for NameServerRemoteNotify in this process.
 */
static Int32 NameServerRemoteNotifyDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 NameServerRemoteNotifyDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the NameServerRemoteNotify driver.
 *
 *  @sa     NameServerRemoteNotifyDrv_close
 */
Int
NameServerRemoteNotifyDrv_open (Void)
{
    Int status      = NAMESERVERREMOTENOTIFY_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NameServerRemoteNotifyDrv_open");

    if (NameServerRemoteNotifyDrv_refCount == 0) {
        NameServerRemoteNotifyDrv_handle = open (
                                             NAMESERVERREMOTENOTIFY_DRIVER_NAME,
                                             O_SYNC | O_RDWR);
        if (NameServerRemoteNotifyDrv_handle < 0) {
            perror (NAMESERVERREMOTENOTIFY_DRIVER_NAME);
            /*! @retval NAMESERVERREMOTENOTIFY_E_OSFAILURE
             *          Failed to open NameServerRemoteNotify driver with OS
             */
            status = NAMESERVERREMOTENOTIFY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotifyDrv_open",
                                 status,
                                 "Failed to open NameServerRemoteNotify driver"
                                 " with OS!");
        }
        else {
            osStatus = fcntl (NameServerRemoteNotifyDrv_handle,
                              F_SETFD,
                              FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval NAMESERVERREMOTENOTIFY_E_OSFAILURE Failed to set
                 *      file descriptor flags
                 */
                status = NAMESERVERREMOTENOTIFY_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServerRemoteNotifyDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }

    if (status == NAMESERVERREMOTENOTIFY_SUCCESS) {
        /* TBD: Protection for refCount. */
        NameServerRemoteNotifyDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotifyDrv_open", status);

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS
     *          Operation successfully completed.
     */
    return status;
}


/*!
 *  @brief  Function to close the NameServerRemoteNotify driver.
 *
 *  @sa     NameServerRemoteNotifyDrv_open
 */
Int
NameServerRemoteNotifyDrv_close (Void)
{
    Int status      = NAMESERVERREMOTENOTIFY_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NameServerRemoteNotifyDrv_close");

    /* TBD: Protection for refCount. */
    NameServerRemoteNotifyDrv_refCount--;
    if (NameServerRemoteNotifyDrv_refCount == 0) {
        osStatus = close (NameServerRemoteNotifyDrv_handle);
        if (osStatus != 0) {
            perror ("NameServerRemoteNotify driver close: ");
            /*! @retval NAMESERVERREMOTENOTIFY_E_OSFAILURE
             *          Failed to open NameServerRemoteNotify driver with OS
             */
            status = NAMESERVERREMOTENOTIFY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotifyDrv_close",
                                 status,
                                 "Failed to close NameServerRemoteNotify driver"
                                 " with OS!");
        }
        else {
            NameServerRemoteNotifyDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotifyDrv_close", status);

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS
     *          Operation successfully completed.
     */
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
NameServerRemoteNotifyDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = NAMESERVERREMOTENOTIFY_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "NameServerRemoteNotifyDrv_ioctl",
               cmd, args);

    GT_assert (curTrace, (NameServerRemoteNotifyDrv_refCount > 0));

    osStatus = ioctl (NameServerRemoteNotifyDrv_handle, cmd, args);
    if (osStatus < 0) {
        /*! @retval NAMESERVERREMOTENOTIFY_E_OSFAILURE Driver ioctl failed */
        status = NAMESERVERREMOTENOTIFY_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotifyDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((NameServerRemoteNotifyDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotifyDrv_ioctl", status);

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS
     *          Operation successfully completed.
     */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
