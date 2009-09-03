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
 *  @file       NotifyDriverShmDrvUsr.c
 *
 *  @brief      User-side OS-specific implementation of NotifyDriverShm driver
 *              for Linux
 *
 */


/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <NotifyDriverShmDrvUsr.h>

/* Module headers */
#include <NotifyDriverShmDrvDefs.h>
#include <NotifyDriverShm.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for NotifyDriverShm.
 */
#define NOTIFYDRIVERSHM_DRIVER_NAME         "/dev/notifyducatidrv"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for NotifyDriverShm in this process.
 */
static Int32 NotifyDriverShm_drvHandle = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the NotifyDriverShm driver.
 *
 *  @sa     NotifyDriverShmDrvUsr_close
 */
Int
NotifyDriverShmDrvUsr_open (Void)
{
    Int status      = NOTIFYDRIVERSHM_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NotifyDriverShmDrvUsr_open");

    /* TBD: Need to handle multi-threading */
    NotifyDriverShm_drvHandle = open (NOTIFYDRIVERSHM_DRIVER_NAME,
                                      O_SYNC | O_RDWR);
    if (NotifyDriverShm_drvHandle < 0) {
        perror ("NotifyDriverShm driver open: " NOTIFYDRIVERSHM_DRIVER_NAME);
        /*! @retval NOTIFYDRIVERSHM_E_OSFAILURE Failed to open NotifyDriverShm
                   driver with OS */
        status = NOTIFYDRIVERSHM_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShmDrvUsr_open",
                             status,
                             "Failed to open NotifyDriverShm driver with OS!");
    }
    else {
        osStatus = fcntl (NotifyDriverShm_drvHandle, F_SETFD, FD_CLOEXEC);
        if (osStatus != 0) {
            /*! @retval NOTIFYDRIVERSHM_E_OSFAILURE Failed to set file
                                     descriptor flags */
            status = NOTIFYDRIVERSHM_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShmDrvUsr_open",
                                 status,
                                 "Failed to set file descriptor flags!");
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShmDrvUsr_open", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the NotifyDriverShm driver.
 *
 *  @sa     NotifyDriverShmDrvUsr_open
 */
Int
NotifyDriverShmDrvUsr_close (Void)
{
    Int status      = NOTIFYDRIVERSHM_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "NotifyDriverShmDrvUsr_close");

    /* TBD: Need to handle multi-threading */
    osStatus = close (NotifyDriverShm_drvHandle);
    if (osStatus != 0) {
        perror ("NotifyDriverShm driver close: " NOTIFYDRIVERSHM_DRIVER_NAME);
        /*! @retval NOTIFYDRIVERSHM_E_OSFAILURE Failed to open NotifyDriverShm
                               driver with OS */
        status = NOTIFYDRIVERSHM_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShmDrvUsr_close",
                             status,
                             "Failed to close NotifyDriverShm driver with OS!");
    }
    else {
        NotifyDriverShm_drvHandle = 0;
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShmDrvUsr_close", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successfully completed. */
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
NotifyDriverShmDrvUsr_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = NOTIFYDRIVERSHM_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "NotifyDriverShmDrvUsr_ioctl", cmd, args);

    osStatus = ioctl (NotifyDriverShm_drvHandle, cmd, args);
    if (osStatus < 0) {
        /*! @retval NOTIFYDRIVERSHM_E_OSFAILURE Driver ioctl failed */
        status = NOTIFYDRIVERSHM_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShmDrvUsr_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((NotifyDriverShm_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShmDrvUsr_ioctl", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successfully completed. */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
