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
 *  @file   MessageQTransportShmDrv.c
 *
 *  @brief      OS-specific implementation of MessageQTransportShm
 *              driver for Linux
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <MessageQTransportShm.h>
#include <MessageQTransportShmDrvDefs.h>

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
 *  @brief  Driver name for MessageQTransportShm.
 */
#define MESSAGEQTRANSPORTSHM_DRIVER_NAME  "/dev/syslinkipc/MessageQTransportShm"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for MessageQTransportShm in this process.
 */
static Int32 MessageQTransportShmDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 MessageQTransportShmDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the MessageQTransportShm driver.
 *
 *  @sa     MessageQTransportShmDrv_close
 */
Int
MessageQTransportShmDrv_open (Void)
{
    Int status      = MESSAGEQTRANSPORTSHM_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MessageQTransportShmDrv_open");

    if (MessageQTransportShmDrv_refCount == 0) {
        MessageQTransportShmDrv_handle = open (MESSAGEQTRANSPORTSHM_DRIVER_NAME,
                                               O_SYNC | O_RDWR);
        if (MessageQTransportShmDrv_handle < 0) {
            perror (MESSAGEQTRANSPORTSHM_DRIVER_NAME);
            /*! @retval MESSAGEQTRANSPORTSHM_E_OSFAILURE
             *          Failed to open MessageQTransportShm driver with OS
             */
            status = MESSAGEQTRANSPORTSHM_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShmDrv_open",
                                 status,
                                 "Failed to open MessageQTransportShm driver"
                                 " with OS!");
        }
        else {
            osStatus = fcntl (MessageQTransportShmDrv_handle,
                              F_SETFD,
                              FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval MESSAGEQTRANSPORTSHM_E_OSFAILURE Failed to set file
                 *          descriptor flags
                 */
                status = MESSAGEQTRANSPORTSHM_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQTransportShmDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }

    if (status == MESSAGEQTRANSPORTSHM_SUCCESS) {
        /* TBD: Protection for refCount. */
        MessageQTransportShmDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShmDrv_open", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS
     *          Operation successfully completed.
     */
    return status;
}


/*!
 *  @brief  Function to close the MessageQTransportShm driver.
 *
 *  @sa     MessageQTransportShmDrv_open
 */
Int
MessageQTransportShmDrv_close (Void)
{
    Int status      = MESSAGEQTRANSPORTSHM_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MessageQTransportShmDrv_close");

    /* TBD: Protection for refCount. */
    MessageQTransportShmDrv_refCount--;
    if (MessageQTransportShmDrv_refCount == 0) {
        osStatus = close (MessageQTransportShmDrv_handle);
        if (osStatus != 0) {
            perror ("MessageQTransportShm driver close: ");
            /*! @retval MESSAGEQTRANSPORTSHM_E_OSFAILURE
             *          Failed to open MessageQTransportShm driver with OS
             */
            status = MESSAGEQTRANSPORTSHM_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShmDrv_close",
                                 status,
                                 "Failed to close MessageQTransportShm driver"
                                 " with OS!");
        }
        else {
            MessageQTransportShmDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShmDrv_close", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS
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
MessageQTransportShmDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = MESSAGEQTRANSPORTSHM_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "MessageQTransportShmDrv_ioctl", cmd, args);

    GT_assert (curTrace, (MessageQTransportShmDrv_refCount > 0));

    osStatus = ioctl (MessageQTransportShmDrv_handle, cmd, args);
    if (osStatus < 0) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_OSFAILURE Driver ioctl failed */
        status = MESSAGEQTRANSPORTSHM_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShmDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((MessageQTransportShmDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShmDrv_ioctl", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS
     *          Operation successfully completed.
     */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
