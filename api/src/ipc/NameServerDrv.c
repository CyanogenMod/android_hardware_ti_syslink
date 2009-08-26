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
 *  @file       NameServerDrv.c
 *
 *  @brief      OS-specific implementation of NameServer driver for Linux
 *
 *
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Gate.h>
#include <NameServer.h>
#include <NameServerDrvDefs.h>

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
 *  @brief  Driver name for NameServer.
 */
#define NAMESERVER_DRIVER_NAME     "/dev/syslinkipc/NameServer"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for NameServer in this process.
 */
static Int32 NameServerDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 NameServerDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the NameServer driver.
 *
 *  @sa     NameServerDrv_close
 */
Int
NameServerDrv_open (Void)
{
    Int status      = NAMESERVER_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NameServerDrv_open");

    if (NameServerDrv_refCount == 0) {
        NameServerDrv_handle = open (NAMESERVER_DRIVER_NAME, O_SYNC | O_RDWR);
        if (NameServerDrv_handle < 0) {
            perror ("NameServer driver open: ");
            /*! @retval NAMESERVER_E_OSFAILURE Failed to open NameServer
             *          driver with OS
             */
            status = NAMESERVER_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerDrv_open",
                                 status,
                                 "Failed to open NameServer driver with OS!");
        }
        else {
            osStatus = fcntl (NameServerDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval NAMESERVER_E_OSFAILURE Failed to set file
                 *          descriptor flags
                 */
                status = NAMESERVER_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServerDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }

    if (status == NAMESERVER_SUCCESS) {
        /* TBD: Protection for refCount. */
        NameServerDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerDrv_open", status);

    /*! @retval NAMESERVER_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the NameServer driver.
 *
 *  @sa     NameServerDrv_open
 */
Int
NameServerDrv_close (Void)
{
    Int status      = NAMESERVER_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NameServerDrv_close");

    /* TBD: Protection for refCount. */
    NameServerDrv_refCount--;
    if (NameServerDrv_refCount == 0) {
        osStatus = close (NameServerDrv_handle);
        if (osStatus != 0) {
            perror ("NameServer driver close: ");
/*! @retval NAMESERVER_E_OSFAILURE Failed to open NameServer driver with OS */
            status = NAMESERVER_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerDrv_close",
                                 status,
                                 "Failed to close NameServer driver with OS!");
        }
        else {
            NameServerDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerDrv_close", status);

/*! @retval NAMESERVER_SUCCESS Operation successfully completed. */
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
NameServerDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = NAMESERVER_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "NameServerDrv_ioctl", cmd, args);

    GT_assert (curTrace, (NameServerDrv_refCount > 0));

    osStatus = ioctl (NameServerDrv_handle, cmd, args);
    if (osStatus < 0) {
    /*! @retval NAMESERVER_E_OSFAILURE Driver ioctl failed */
        status = NAMESERVER_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((NameServerDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerDrv_ioctl", status);

/*! @retval NAMESERVER_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
